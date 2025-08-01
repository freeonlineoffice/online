// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* See CanvasSectionContainer.ts for explanations. */

// We will keep below definitions until we use tsconfig.json.
declare var L: any;

namespace lool {

export class ScrollSection extends CanvasSectionObject {
	// Scrolling animation constants. Unlabelled units are fractions of the line
	// height, so that they're somewhat DPI-independent.
	static readonly scrollAnimationAcceleration: number = 0.2;
	static readonly scrollAnimationMaxVelocity: number = 2.5;
	static readonly scrollAnimationMaxDelta: number = 75;
	static readonly scrollDirectTimeoutMs: number = 100;

	// The number of lines a scroll-wheel tick should travel
	static readonly scrollWheelDelta: number = 20;

	name: string = L.CSections.Scroll.name;
	processingOrder: number = L.CSections.Scroll.processingOrder
	drawingOrder: number = L.CSections.Scroll.drawingOrder;
	zIndex: number = L.CSections.Scroll.zIndex;
	windowSection: boolean = true; // This section covers the entire canvas.

	map: any;
	autoScrollTimer: any;
	pendingScrollEvent: any = null;
	stepByStepScrolling: boolean = false; // quick scroll will move "page up/down" not "jump to"

	isRTL: () => boolean;

	constructor (isRTL?: () => boolean) {
		super();

		this.map = L.Map.THIS;

		this.isRTL = isRTL ?? (() => false);

		this.map.on('scrollby', this.onScrollBy, this);
		this.map.on('scrollvelocity', this.onScrollVelocity, this);
		this.map.on('handleautoscroll', this.onHandleAutoScroll, this);
	}

	public onInitialize (): void {
		this.sectionProperties.docLayer = this.map._docLayer;
		this.sectionProperties.mapPane = (<HTMLElement>(document.querySelectorAll('.leaflet-map-pane')[0]));
		this.sectionProperties.defaultCursorStyle = this.sectionProperties.mapPane.style.cursor;

		this.sectionProperties.yMax = 0;
		this.sectionProperties.yMin = 0;
		this.sectionProperties.xMax = 0;
		this.sectionProperties.xMin = 0;

		this.sectionProperties.previousDragDistance = null;

		this.sectionProperties.usableThickness = 20 * app.roundedDpiScale;
		this.sectionProperties.scrollBarThickness = 6 * app.roundedDpiScale;
		this.sectionProperties.edgeOffset = 0;

		this.sectionProperties.drawScrollBarRailway = true;
		this.sectionProperties.scrollBarRailwayThickness = 12 * app.roundedDpiScale;
		this.sectionProperties.scrollBarRailwayAlpha = this.map._docLayer._docType === 'spreadsheet' ? 1.0 : 0.5;
		this.sectionProperties.scrollBarRailwayColor = '#EFEFEF';

		this.sectionProperties.drawVerticalScrollBar = ((<any>window).mode.isDesktop() ? true: false);
		this.sectionProperties.drawHorizontalScrollBar = ((<any>window).mode.isDesktop() ? true: false);

		this.sectionProperties.clickScrollVertical = false; // true when user presses on the scroll bar drawing.
		this.sectionProperties.clickScrollHorizontal = false;

		this.sectionProperties.mouseIsOnVerticalScrollBar = false;
		this.sectionProperties.mouseIsOnHorizontalScrollBar = false;

		this.sectionProperties.minimumScrollSize = 80 * app.roundedDpiScale;

		this.sectionProperties.circleSliderRadius = 24 * app.roundedDpiScale; // Radius of the mobile vertical circular slider.
		this.sectionProperties.arrowCornerLength = 10 * app.roundedDpiScale; // Corner length of the arrows inside circular slider.

		// Opacity.
		this.sectionProperties.alphaWhenVisible = 0.5; // Scroll bar is visible but not being used.
		this.sectionProperties.alphaWhenBeingUsed = 0.8; // Scroll bar is being used.
		this.sectionProperties.currentAlpha = 1.0; // This variable will be updated while animating. When not animating, this will be equal to one of the above variables.

		// Durations.
		this.sectionProperties.idleDuration = 2000; // In milliseconds. Scroll bar will be visible for this period of time after being used.
		this.sectionProperties.fadeOutStartingTime = 1800; // After this period, scroll bar starts to disappear. This duration is included in "idleDuration".
		this.sectionProperties.fadeOutDuration = this.sectionProperties.idleDuration - this.sectionProperties.fadeOutStartingTime;

		this.sectionProperties.yOffset = 0;
		this.sectionProperties.xOffset = 0;

		this.sectionProperties.horizontalScrollRightOffset = this.sectionProperties.usableThickness * 2; // To prevent overlapping of the scroll bars.

		this.sectionProperties.animatingVerticalScrollBar = false;
		this.sectionProperties.animatingHorizontalScrollBar = false;
		this.sectionProperties.animatingScroll = false;

		this.sectionProperties.animateWheelScroll = (<any>window).mode.isDesktop();
		this.sectionProperties.lastElapsedTime = 0;
		this.sectionProperties.scrollAnimationDelta = [0, 0];
		this.sectionProperties.scrollAnimationAcc = [0, 0];
		this.sectionProperties.scrollAnimationVelocity = [0, 0];
		this.sectionProperties.scrollAnimationDisableTimeout = null;
		this.sectionProperties.scrollWheelDelta = [0, 0];	// Used for non-animated scrolling

		this.sectionProperties.pointerSyncWithVerticalScrollBar = true;
		this.sectionProperties.pointerSyncWithHorizontalScrollBar = true;
		this.sectionProperties.pointerReCaptureSpacer = null; // Clicked point of the scroll bar.

		// Step by step scrolling interval in ms
		this.sectionProperties.stepDuration = 50;
		this.sectionProperties.quickScrollHorizontalTimer = null;

		// Chrome's mouse-wheel events are extremely inconsistent between platforms and they don't
		// use the deltaMode property, so we need to apply heuristics to determine when a wheel event
		// comes from a mouse-wheel or a touchpad.
		this.sectionProperties.scrollQuirks = true;

		this.sectionProperties.moveMapBy = null; // Move map this amount [x, y] (CSS pixels) when updating the DOM.
	}

	public completePendingScroll(): void {
		if (this.pendingScrollEvent) {
			this.onScrollTo(this.pendingScrollEvent, true /* force */);
			this.pendingScrollEvent = null;
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onScrollTo (e: any, force: boolean = false): void {
		if (!force && !this.containerObject.drawingAllowed()) {
			// Only remember the last scroll-to position.
			this.pendingScrollEvent = e;
			return;
		}
		// Triggered by the document (e.g. search result out of the viewing area).
		if (this.map.panBy) {
			this.moveMapBy(e.x - app.file.viewedRectangle.cX1, e.y - app.file.viewedRectangle.cY1, true);
		}
	}

	public moveMapBy(cX: number, cY: number, reset: boolean = false): void {
		if (this.sectionProperties.moveMapBy !== null) {
			this.sectionProperties.moveMapBy[0] = reset === false ? this.sectionProperties.moveMapBy[0] + cX : cX;
			this.sectionProperties.moveMapBy[1] = reset === false ? this.sectionProperties.moveMapBy[1] + cY : cY;
		}
		else {
			this.sectionProperties.moveMapBy = [cX, cY];
		}

		this.containerObject.requestReDraw();
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onScrollBy (e: any): void {
		if (this.map._docLayer._docType !== 'spreadsheet') {
			this.scrollVerticalWithOffset(e.y);
			this.scrollHorizontalWithOffset(e.x);
		} else {
			// For Calc, top position shouldn't be below zero, for others, we can activate a similar check if needed (while keeping in mind that top position may be below zero for others).
			var docTopLef = this.containerObject.getDocumentTopLeft();

			// Some early exits.
			if (e.y < 0 && docTopLef[1] === 0) // Don't scroll to negative values.
				return;

			if (e.x < 0 && docTopLef[0] === 0)
				return;

			var diff = Math.round(e.y * app.dpiScale);

			if (docTopLef[1] + diff < 0) {
				e.y = Math.round(-1 * docTopLef[1] / app.dpiScale);
			}

			diff = Math.round(e.x * app.dpiScale);
			if (docTopLef[0] + diff < 0) {
				e.x = Math.round(-1 * docTopLef[0] / app.dpiScale);
			}

			this.moveMapBy(e.x, e.y);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onScrollVelocity (e: any): void {
		if (e.vx === 0 && e.vy === 0) {
			clearInterval(this.autoScrollTimer);
			this.autoScrollTimer = null;
			this.map.isAutoScrolling = false;
		} else {
			clearInterval(this.autoScrollTimer);
			this.map.isAutoScrolling = true;
			this.autoScrollTimer = setInterval(L.bind(function() {
				this.onScrollBy({x: e.vx, y: e.vy});
				// Unfortunately, dragging outside the map doesn't work for the map element.
				// We will keep this until we remove leaflet.
				if (L.Map.THIS.mouse
				&& L.Map.THIS.mouse._mouseDown
				&& this.containerObject.targetBoundSectionListContains(L.CSections.Tiles.name)
				&& (<any>window).mode.isDesktop()
				&& this.containerObject.isDraggingSomething()
				&& L.Map.THIS._docLayer._docType === 'spreadsheet') {
					var temp = [e.pos.x, e.pos.y];
					var tempPos = [(this.isRTL() ? this.map._size.x - temp[0] : temp[0]) * app.dpiScale, temp[1] * app.dpiScale];
					var docTopLeft = app.sectionContainer.getDocumentTopLeft();
					tempPos = [tempPos[0] + docTopLeft[0], tempPos[1] + docTopLeft[1]];
					tempPos = [Math.round(tempPos[0] * app.pixelsToTwips), Math.round(tempPos[1] * app.pixelsToTwips)];
					L.Map.THIS._docLayer._postMouseEvent('move', tempPos[0], tempPos[1], 1, 1, 0);
				}
			}, this), 100);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onHandleAutoScroll (e :any): void {
		var vx = 0;
		var vy = 0;

		if (e.pos.y > e.map._size.y - 50) {
			vy = 50;
		} else if (e.pos.y < 50 && e.map._getTopLeftPoint().y > 50) {
			vy = -50;
		}

		const mousePosX: number = this.isRTL() ? e.map._size.x - e.pos.x : e.pos.x;
		const mapLeft: number = this.isRTL() ? e.map._size.x - e.map._getTopLeftPoint().x : e.map._getTopLeftPoint().x;
		if (mousePosX > e.map._size.x - 50) {
			vx = 50;
		} else if (mousePosX < 50 && mapLeft > 50) {
			vx = -50;
		}

		this.onScrollVelocity({ vx: vx, vy: vy, pos: e.pos });
	}

	private getVerticalScrollLength (): number {
		var result: number = this.containerObject.getDocumentAnchorSection().size[1];
		this.sectionProperties.yOffset = this.containerObject.getDocumentAnchorSection().myTopLeft[1];

		if (this.map._docLayer._docType !== 'spreadsheet') {
			return result;
		}
		else {
			var splitPanesContext: any = this.map.getSplitPanesContext();
			var splitPos = {x: 0, y: 0};
			if (splitPanesContext) {
				splitPos = splitPanesContext.getSplitPos().clone();
				splitPos.y = Math.round(splitPos.y * app.dpiScale);
			}

			this.sectionProperties.yOffset += splitPos.y;
			return result - splitPos.y;
		}
	}

	private calculateVerticalScrollSize (scrollLength: number) :number {
		var scrollSize = Math.round(scrollLength * scrollLength / app.view.size.pY);
		return Math.round(scrollSize);
	}

	private calculateYMinMax () {
		var diff: number = Math.round(app.view.size.pY - this.containerObject.getDocumentAnchorSection().size[1]);

		if (diff >= 0) {
			this.sectionProperties.yMin = 0;
			this.sectionProperties.yMax = diff;
			if ((<any>window).mode.isDesktop())
				this.sectionProperties.drawVerticalScrollBar = true;
		}
		else {
			diff = Math.round((app.view.size.pY - this.containerObject.getDocumentAnchorSection().size[1]) * 0.5);
			this.sectionProperties.yMin = app.map.getDocType() === 'spreadsheet' ? 0 : diff;
			this.sectionProperties.yMax = diff;
			if (app.view.size.pY > 0) {
				if (this.map._docLayer._docType !== 'spreadsheet' || !(<any>window).mode.isDesktop())
					this.sectionProperties.drawVerticalScrollBar = false;
			}
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public getVerticalScrollProperties (): any {
		this.calculateYMinMax();
		var result: any = {};
		result.scrollLength = this.getVerticalScrollLength(); // The length of the railway that the scroll bar moves on up & down.
		result.scrollSize = this.calculateVerticalScrollSize(result.scrollLength); // Size of the scroll bar.

		if (result.scrollSize < this.sectionProperties.minimumScrollSize) {
			var diff: number = this.sectionProperties.minimumScrollSize - result.scrollSize;
			result.scrollLength -= diff;
			result.scrollSize = this.sectionProperties.minimumScrollSize;
		}

		result.ratio = app.view.size.pY / result.scrollLength; // 1px scrolling = xpx document height.
		result.startY = Math.round(this.documentTopLeft[1] / result.ratio + this.sectionProperties.yOffset);

		result.verticalScrollStep = this.size[1] / 2;

		return result;
	}

	private getHorizontalScrollLength (): number {
		var result: number = this.containerObject.getDocumentAnchorSection().size[0];
		this.sectionProperties.xOffset = this.containerObject.getDocumentAnchorSection().myTopLeft[0];

		if (this.map._docLayer._docType !== 'spreadsheet') {
			return result - this.sectionProperties.horizontalScrollRightOffset;
		}
		else {
			var splitPanesContext: any = this.map.getSplitPanesContext();
			var splitPos = {x: 0, y: 0};
			if (splitPanesContext) {
				splitPos = splitPanesContext.getSplitPos().clone();
				splitPos.x = Math.round(splitPos.x * app.dpiScale);
			}

			this.sectionProperties.xOffset += splitPos.x;
			return result - splitPos.x - this.sectionProperties.horizontalScrollRightOffset;
		}
	}

	private calculateHorizontalScrollSize (scrollLength: number): number {
		var scrollSize = Math.round(scrollLength * scrollLength / app.view.size.pX);
		return scrollSize;
	}

	private calculateXMinMax (): void {
		var diff: number = Math.round(app.view.size.pX - this.containerObject.getDocumentAnchorSection().size[0]);

		if (diff >= 0) {
			this.sectionProperties.xMin = 0;
			this.sectionProperties.xMax = diff;
			if ((<any>window).mode.isDesktop())
				this.sectionProperties.drawHorizontalScrollBar = true;
		}
		else {
			diff = Math.round((app.view.size.pX - this.containerObject.getDocumentAnchorSection().size[0]) * 0.5);
			this.sectionProperties.xMin = diff;
			this.sectionProperties.xMax = diff;
			if (app.view.size.pX >  0) {
				if (this.map._docLayer._docType !== 'spreadsheet' || !(<any>window).mode.isDesktop())
					this.sectionProperties.drawHorizontalScrollBar = false;
			}
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public getHorizontalScrollProperties (): any {
		this.calculateXMinMax();
		var result: any = {};
		result.scrollLength = this.getHorizontalScrollLength(); // The length of the railway that the scroll bar moves on left & right.
		result.scrollSize = this.calculateHorizontalScrollSize(result.scrollLength); // Width of the scroll bar.

		if (result.scrollSize < this.sectionProperties.minimumScrollSize) {
			var diff: number = this.sectionProperties.minimumScrollSize - result.scrollSize;
			result.scrollLength -= diff;
			result.scrollSize = this.sectionProperties.minimumScrollSize;
		}

		result.ratio = app.view.size.pX / result.scrollLength;
		result.startX = Math.round(this.documentTopLeft[0] / result.ratio + this.sectionProperties.xOffset);

		result.horizontalScrollStep = this.size[0] / 2;

		return result;
	}

	public onUpdateScrollOffset (): void {
		if (this.map._docLayer._docType === 'spreadsheet') {
			this.map._docLayer.refreshViewData();
			this.map._docLayer._restrictDocumentSize();
		}
	}

	private DrawVerticalScrollBarMobile (): void {
		var scrollProps: any = this.getVerticalScrollProperties();

		if (this.sectionProperties.animatingVerticalScrollBar)
			this.context.globalAlpha = this.sectionProperties.currentAlpha;
		else
			this.context.globalAlpha = this.sectionProperties.clickScrollVertical ? this.sectionProperties.alphaWhenBeingUsed: this.sectionProperties.alphaWhenVisible;

		this.context.strokeStyle = '#7E8182';
		this.context.fillStyle = 'white';

		var circleStartY = scrollProps.startY + this.sectionProperties.circleSliderRadius;
		var circleStartX = this.isRTL()
			? this.sectionProperties.circleSliderRadius * 0.5
			: this.size[0] - this.sectionProperties.circleSliderRadius * 0.5;

		this.context.beginPath();
		this.context.arc(circleStartX, circleStartY, this.sectionProperties.circleSliderRadius, 0, Math.PI * 2, true);
		this.context.fill();
		this.context.stroke();

		this.context.fillStyle = '#7E8182';
		this.context.beginPath();
		var x: number = circleStartX - this.sectionProperties.arrowCornerLength * 0.5;
		var y: number = circleStartY - 5 * app.roundedDpiScale;
		this.context.moveTo(x, y);
		x += this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y -= Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y += Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		this.context.fill();

		x = circleStartX - this.sectionProperties.arrowCornerLength * 0.5;
		y = circleStartY + 5 * app.roundedDpiScale;
		this.context.moveTo(x, y);
		x += this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y += Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y -= Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		this.context.fill();

		this.context.globalAlpha = 1.0;
	}

	private drawVerticalScrollBar (): void {
		var scrollProps: any = this.getVerticalScrollProperties();
		const isDarkBackground = this.map.uiManager.isBackgroundDark();
		const docType = this.sectionProperties.docLayer._docType;

		var startX = this.isRTL() ? this.sectionProperties.edgeOffset : this.size[0] - this.sectionProperties.scrollBarThickness - this.sectionProperties.edgeOffset;

		if (isDarkBackground && (docType === 'text' || docType === 'drawing')) {
			this.sectionProperties.scrollBarRailwayColor = 'transparent';
		}

		if (this.sectionProperties.drawScrollBarRailway) {
			this.context.globalAlpha = this.sectionProperties.scrollBarRailwayAlpha;
			this.context.fillStyle = this.sectionProperties.scrollBarRailwayColor;
			this.context.fillRect(
				startX,
				this.sectionProperties.yMin + this.sectionProperties.yOffset,
				this.sectionProperties.scrollBarRailwayThickness,
				this.sectionProperties.yMax - this.sectionProperties.yMin - this.sectionProperties.yOffset
			);
		}

		if (this.sectionProperties.animatingVerticalScrollBar) {
			this.context.globalAlpha = this.sectionProperties.currentAlpha;
		} else {
			this.context.globalAlpha = this.sectionProperties.clickScrollVertical ? this.sectionProperties.alphaWhenBeingUsed: this.sectionProperties.alphaWhenVisible;
		}

		this.context.fillStyle = '#7E8182';


		this.context.fillRect(startX, scrollProps.startY, this.sectionProperties.scrollBarThickness, scrollProps.scrollSize - this.sectionProperties.scrollBarThickness);

		this.context.globalAlpha = 1.0;

		if (this.containerObject.testing) {
			var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-vertical-scrollbar');
			if (!element) {
				element = document.createElement('div');
				element.id = 'test-div-vertical-scrollbar';
				document.body.appendChild(element);
			}
			element.textContent = String(scrollProps.startY);
			element.style.display = 'none';
			element.style.position = 'fixed';
			element.style.zIndex = '-1';
		}
	}

	private drawHorizontalScrollBar (): void {
		var scrollProps: any = this.getHorizontalScrollProperties();

		var startY = this.size[1] - this.sectionProperties.scrollBarThickness - this.sectionProperties.edgeOffset;

		const sizeX = scrollProps.scrollSize - this.sectionProperties.scrollBarThickness;
		const docWidth: number = this.map.getPixelBoundsCore().getSize().x;
		const startX = this.isRTL() ? docWidth - scrollProps.startX - sizeX : scrollProps.startX;

		if (this.sectionProperties.drawScrollBarRailway) {
			this.context.globalAlpha = this.sectionProperties.scrollBarRailwayAlpha;
			this.context.fillStyle = this.sectionProperties.scrollBarRailwayColor;
			this.context.fillRect(
				this.sectionProperties.xMin + this.sectionProperties.xOffset,
				startY,
				this.sectionProperties.xMax - this.sectionProperties.xMin - this.sectionProperties.xOffset,
				this.sectionProperties.scrollBarRailwayThickness
			);
		}

		if (this.sectionProperties.animatingHorizontalScrollBar)
			this.context.globalAlpha = this.sectionProperties.currentAlpha;
		else
			this.context.globalAlpha = this.sectionProperties.clickScrollHorizontal ? this.sectionProperties.alphaWhenBeingUsed: this.sectionProperties.alphaWhenVisible;

		this.context.fillStyle = '#7E8182';


		this.context.fillRect(startX, startY, sizeX, this.sectionProperties.scrollBarThickness);

		this.context.globalAlpha = 1.0;

		if (this.containerObject.testing) {
			var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-horizontal-scrollbar');
			if (!element) {
				element = document.createElement('div');
				element.id = 'test-div-horizontal-scrollbar';
				document.body.appendChild(element);
			}
			element.textContent = String(scrollProps.startX);
			element.style.display = 'none';
			element.style.position = 'fixed';
			element.style.zIndex = '-1';
		}

	}

	private calculateCurrentAlpha (elapsedTime: number): void {
		if (elapsedTime >= this.sectionProperties.fadeOutStartingTime) {
			this.sectionProperties.currentAlpha = Math.max((1 - ((elapsedTime - this.sectionProperties.fadeOutStartingTime) / this.sectionProperties.fadeOutDuration)) * this.sectionProperties.alphaWhenVisible, 0.1);
		}
		else {
			this.sectionProperties.currentAlpha = this.sectionProperties.alphaWhenVisible;
		}
	}

	private doMove() {
		app.layoutingService.appendLayoutingTask(() => {
			this.map.panBy(new L.Point(this.sectionProperties.moveMapBy[0], this.sectionProperties.moveMapBy[1]));
			this.sectionProperties.moveMapBy = null;
			this.onUpdateScrollOffset();

			if (app && app.file.fileBasedView === true)
				app.map._docLayer._checkSelectedPart();
		});
	}

	public onDraw (frameCount: number, elapsedTime: number): void {
		if (this.isAnimating && frameCount >= 0)
			this.calculateCurrentAlpha(elapsedTime);

		if ((this.sectionProperties.drawVerticalScrollBar || this.sectionProperties.animatingVerticalScrollBar)) {
			if ((<any>window).mode.isMobile())
				this.DrawVerticalScrollBarMobile();
			else
				this.drawVerticalScrollBar();
		}

		if ((this.sectionProperties.drawHorizontalScrollBar || this.sectionProperties.animatingHorizontalScrollBar)) {
			this.drawHorizontalScrollBar();
		}

		if (this.sectionProperties.moveMapBy !== null) {
			this.doMove();
		}
	}

	public onAnimate(frameCount: number, elapsedTime: number): void {
		if (this.sectionProperties.animatingScroll) {
			const lineHeight = this.containerObject.getScrollLineHeight();
			// Smoothness will be affected by Firefox bug #1967935
			const timeDelta = (elapsedTime - this.sectionProperties.lastElapsedTime) / (1000/60);
			const accel = lineHeight * ScrollSection.scrollAnimationAcceleration * timeDelta * app.dpiScale;
			const maxVelocity = lineHeight * ScrollSection.scrollAnimationMaxVelocity * timeDelta * app.dpiScale;
			const maxDelta = lineHeight * ScrollSection.scrollAnimationMaxDelta * app.dpiScale;

			// Calculate horizontal and vertical scroll deltas for this animation step
			const deltas = [0, 0];
			for (let i = 0; i < 2; ++i) {
				this.sectionProperties.scrollAnimationAcc[i] += this.sectionProperties.scrollAnimationDelta[i];
				this.sectionProperties.scrollAnimationDelta[i] = 0;

				// Don't let the accumulated delta get too big, or the user will be able to
				// accumulate a long scrolling animation and it'd feel weird.
				const sign = this.sectionProperties.scrollAnimationAcc[i] > 0 ? 1 : -1;
				if (Math.abs(this.sectionProperties.scrollAnimationAcc[i]) > maxDelta)
					this.sectionProperties.scrollAnimationAcc[i] = sign * maxDelta;
				this.sectionProperties.scrollAnimationVelocity[i] += accel * sign;

				deltas[i] = Math.round(Math.min(Math.abs(this.sectionProperties.scrollAnimationVelocity[i]), maxVelocity) * sign);
				if (Math.abs(deltas[i]) >= Math.abs(this.sectionProperties.scrollAnimationAcc[i])) {
					deltas[i] = this.sectionProperties.scrollAnimationAcc[i];
					this.sectionProperties.scrollAnimationAcc[i] = 0;
					this.sectionProperties.scrollAnimationVelocity[i] = 0;
				} else
					this.sectionProperties.scrollAnimationAcc[i] -= deltas[i];
			}

			// Perform scrolling, if necessary
			if (deltas[0] !== 0)
				this.scrollHorizontalWithOffset(deltas[0]);
			if (deltas[1] !== 0)
				this.scrollVerticalWithOffset(deltas[1]);
		} else {
			if (this.sectionProperties.scrollWheelDelta[0] !== 0) {
				const delta = this.sectionProperties.scrollWheelDelta[0];
				this.sectionProperties.scrollWheelDelta[0] = 0;
				this.scrollHorizontalWithOffset(delta);
			}
			if (this.sectionProperties.scrollWheelDelta[1] !== 0) {
				const delta = this.sectionProperties.scrollWheelDelta[1];
				this.sectionProperties.scrollWheelDelta[1] = 0;
				this.scrollVerticalWithOffset(delta);
			}
		}

		this.sectionProperties.lastElapsedTime = elapsedTime;

		const animatingScrollbar =
			(this.sectionProperties.animatingHorizontalScrollBar
			|| this.sectionProperties.animatingVerticalScrollBar) &&
			elapsedTime && (elapsedTime < this.sectionProperties.fadeOutDuration);
		const animatingScroll = this.sectionProperties.animatingScroll
			&& this.sectionProperties.scrollAnimationAcc.reduce((a: number, x: number) => a + x, 0) !== 0;
		if (!animatingScrollbar && !animatingScroll) this.containerObject.stopAnimating();
	}

	public onAnimationEnded (frameCount: number, elapsedTime: number): void {
		this.sectionProperties.animatingVerticalScrollBar = false;
		this.sectionProperties.animatingHorizontalScrollBar = false;
		this.sectionProperties.animatingScroll = false;
		this.sectionProperties.scrollAnimationAcc = [0, 0];
		this.sectionProperties.scrollAnimationVelocity = [0, 0];
	}

	private fadeOutHorizontalScrollBar (): void {
		if (this.isAnimating) {
			this.resetAnimation();
			this.sectionProperties.animatingHorizontalScrollBar = true;
		}
		else {
			var options: any = {
				duration: this.sectionProperties.idleDuration,
			};

			this.sectionProperties.animatingHorizontalScrollBar = this.startAnimating(options);
		}
	}

	private fadeOutVerticalScrollBar (): void {
		if (this.isAnimating) {
			this.resetAnimation();
			this.sectionProperties.animatingVerticalScrollBar = true;
		}
		else {
			var options: any = {
				duration: this.sectionProperties.idleDuration,
			};

			this.sectionProperties.animatingVerticalScrollBar = this.startAnimating(options);
		}
	}

	private increaseScrollBarThickness () : void {
		this.sectionProperties.scrollBarThickness = 8 * app.roundedDpiScale;
		this.containerObject.requestReDraw();
	}

	private decreaseScrollBarThickness () : void {
		this.sectionProperties.scrollBarThickness = 6 * app.roundedDpiScale;
		this.containerObject.requestReDraw();
	}

	private hideVerticalScrollBar (): void {
		if (this.sectionProperties.mouseIsOnVerticalScrollBar) {
			this.sectionProperties.mouseIsOnVerticalScrollBar = false;
			this.sectionProperties.mapPane.style.cursor = this.sectionProperties.defaultCursorStyle;

			this.decreaseScrollBarThickness();

			if (!(<any>window).mode.isDesktop()) { // On desktop, we don't want to hide the vertical scroll bar.
				this.sectionProperties.drawVerticalScrollBar = false;
				this.fadeOutVerticalScrollBar();
			}

			// just in case if we have blinking cursor visible
			// we need to change cursor from default style
			if (this.map._docLayer._cursorMarker)
				this.map._docLayer._cursorMarker.setMouseCursor();
		}
	}

	private showVerticalScrollBar (): void {
		if (this.isAnimating && this.sectionProperties.animatingVerticalScrollBar)
			this.containerObject.stopAnimating();

		if (!this.sectionProperties.mouseIsOnVerticalScrollBar) {
			this.sectionProperties.drawVerticalScrollBar = true;
			this.sectionProperties.mouseIsOnVerticalScrollBar = true;
			this.sectionProperties.mapPane.style.cursor = 'pointer';

			// Prevent Instant Mouse hover
			setTimeout(() => {
				if (this.sectionProperties.mouseIsOnVerticalScrollBar) {
					this.increaseScrollBarThickness();
				}
			}, 100);

			if (!this.containerObject.isDraggingSomething() && !(<any>window).mode.isDesktop())
				this.containerObject.requestReDraw();
		}
	}

	private hideHorizontalScrollBar (): void {
		if (this.sectionProperties.mouseIsOnHorizontalScrollBar) {
			this.sectionProperties.mouseIsOnHorizontalScrollBar = false;
			this.sectionProperties.mapPane.style.cursor = this.sectionProperties.defaultCursorStyle;

			this.decreaseScrollBarThickness();

			if (!(<any>window).mode.isDesktop()) {
				this.sectionProperties.drawHorizontalScrollBar = false;
				this.fadeOutHorizontalScrollBar();
			}

			// just in case if we have blinking cursor visible
			// we need to change cursor from default style
			if (this.map._docLayer._cursorMarker)
				this.map._docLayer._cursorMarker.setMouseCursor();
		}
	}

	private showHorizontalScrollBar (): void {
		if (this.isAnimating && this.sectionProperties.animatingHorizontalScrollBar)
			this.containerObject.stopAnimating();

		if (!this.sectionProperties.mouseIsOnHorizontalScrollBar) {
			this.sectionProperties.drawHorizontalScrollBar = true;
			this.sectionProperties.mouseIsOnHorizontalScrollBar = true;
			this.sectionProperties.mapPane.style.cursor = 'pointer';

			// Prevent Instant Mouse hover
			setTimeout(() => {
				if (this.sectionProperties.mouseIsOnHorizontalScrollBar) {
					this.increaseScrollBarThickness();
				}
			}, 100);

			if (!this.containerObject.isDraggingSomething() && !(<any>window).mode.isDesktop())
				this.containerObject.requestReDraw();
		}
	}

	private isMouseOnScrollBar (point: lool.SimplePoint): void {
		const mirrorX = this.isRTL();
		if (this.documentTopLeft[1] >= 0) {
			if ((!mirrorX && point.pX >= this.size[0] - this.sectionProperties.usableThickness)
				|| (mirrorX && point.pX <= this.sectionProperties.usableThickness)) {
				if (point.pY > this.sectionProperties.yOffset) {
					this.showVerticalScrollBar();
				}
				else {
					this.hideVerticalScrollBar();
				}
			}
			else {
				this.hideVerticalScrollBar();
			}
		}

		if (this.documentTopLeft[0] >= 0) {
			if (point.pY >= this.size[1] - this.sectionProperties.usableThickness) {
				if ((!mirrorX && point.pX <= this.size[0] - this.sectionProperties.horizontalScrollRightOffset && point.pX >= this.sectionProperties.xOffset)
					|| (mirrorX && point.pX >= this.sectionProperties.horizontalScrollRightOffset && point.pX >= this.sectionProperties.xOffset)) {
					this.showHorizontalScrollBar();
				}
				else {
					this.hideHorizontalScrollBar();
				}
			}
			else {
				this.hideHorizontalScrollBar();
			}
		}
	}

	public onMouseLeave (): void {
		this.hideVerticalScrollBar();
		this.hideHorizontalScrollBar();
	}

	public scrollVerticalWithOffset (offset: number): boolean {
		this.calculateYMinMax();

		let control = offset;

		if (this.sectionProperties.moveMapBy)
			control += this.sectionProperties.moveMapBy[1]; // Add pending offset.

		if (offset > 0) {
			if (this.documentTopLeft[1] + control > this.sectionProperties.yMax)
				offset = this.sectionProperties.yMax - this.documentTopLeft[1] - control;
			if (offset <= 0)
				return false;
		}
		else {
			if (this.documentTopLeft[1] + control < this.sectionProperties.yMin)
				offset = this.sectionProperties.yMin - this.documentTopLeft[1] - control;
			if (offset >= 0)
				return false;
		}

		this.moveMapBy(0, offset / app.dpiScale);

		if (app.file.fileBasedView) this.map._docLayer._checkSelectedPart();

		if (!this.sectionProperties.drawVerticalScrollBar) {
			if (this.isAnimating) {
				this.resetAnimation();
				this.sectionProperties.animatingVerticalScrollBar = true;
			}
			else
				this.fadeOutVerticalScrollBar();
		}

		return true;
	}

	public scrollHorizontalWithOffset (offset: number): boolean {
		this.calculateXMinMax();

		let control = offset;

		if (this.sectionProperties.moveMapBy)
			control += this.sectionProperties.moveMapBy[0]; // Add pending offset.

		if (this.isRTL()) offset = -offset;

		if (offset > 0) {
			if (this.documentTopLeft[0] + control > this.sectionProperties.xMax)
				offset = this.sectionProperties.xMax - this.documentTopLeft[0] - control;
			if (offset <= 0)
				return false;
		}
		else {
			if (this.documentTopLeft[0] + control < this.sectionProperties.xMin)
				offset = this.sectionProperties.xMin - this.documentTopLeft[0] - control;
			if (offset >= 0)
				return false;
		}

		this.moveMapBy(offset / app.dpiScale, 0);

		if (!this.sectionProperties.drawHorizontalScrollBar) {
			if (this.isAnimating) {
				this.resetAnimation();
				this.sectionProperties.animatingHorizontalScrollBar = true;
			}
			else
				this.fadeOutHorizontalScrollBar();
		}

		return true;
	}

	private isMouseInsideDocumentAnchor (point: lool.SimplePoint): boolean {
		var docSection = this.containerObject.getDocumentAnchorSection();
		return this.containerObject.doesSectionIncludePoint(docSection, point.pToArray());
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private isMousePointerSyncedWithVerticalScrollBar (scrollProps: any, position: lool.SimplePoint): boolean {
		// Keep this desktop-only for now.
		if (!(<any>window).mode.isDesktop())
			return true;

		var spacer = 0;
		if (!this.sectionProperties.pointerSyncWithVerticalScrollBar) {
			spacer = this.sectionProperties.pointerReCaptureSpacer;
		}

		var pointerIsSyncWithScrollBar = false;
		if (this.sectionProperties.pointerSyncWithVerticalScrollBar) {
			pointerIsSyncWithScrollBar = scrollProps.startY < position.pX && scrollProps.startY + scrollProps.scrollSize - this.sectionProperties.scrollBarThickness > position.pY;
			pointerIsSyncWithScrollBar = pointerIsSyncWithScrollBar || (this.isMouseInsideDocumentAnchor(position) && spacer === 0);
		}
		else {
			// See if the scroll bar is on top or bottom.
			var docAncSectionY = this.containerObject.getDocumentAnchorSection().myTopLeft[1];
			if (scrollProps.startY < 30 * window.app.roundedDpiScale + docAncSectionY) {
				pointerIsSyncWithScrollBar = scrollProps.startY + spacer < position.pY;
			}
			else {
				pointerIsSyncWithScrollBar = scrollProps.startY + spacer > position.pY;
			}
		}

		this.sectionProperties.pointerSyncWithVerticalScrollBar = pointerIsSyncWithScrollBar;
		return pointerIsSyncWithScrollBar;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private isMousePointerSyncedWithHorizontalScrollBar (scrollProps: any, position: lool.SimplePoint): boolean {
		// Keep this desktop-only for now.
		if (!(<any>window).mode.isDesktop())
			return true;

		var spacer = 0;
		if (!this.sectionProperties.pointerSyncWithHorizontalScrollBar) {
			spacer = this.sectionProperties.pointerReCaptureSpacer;
		}

		const sizeX = scrollProps.scrollSize - this.sectionProperties.scrollBarThickness;
		const docWidth: number = this.map.getPixelBoundsCore().getSize().x;
		const startX = this.isRTL() ? docWidth - scrollProps.startX - sizeX : scrollProps.startX;
		const endX = startX + sizeX;

		var pointerIsSyncWithScrollBar = false;
		if (this.sectionProperties.pointerSyncWithHorizontalScrollBar) {
			pointerIsSyncWithScrollBar = position.pX > startX && position.pX < endX;
			pointerIsSyncWithScrollBar = pointerIsSyncWithScrollBar || (this.isMouseInsideDocumentAnchor(position) && spacer === 0);
		}
		else {
			// See if the scroll bar is on left or right.
			var docAncSectionX = this.containerObject.getDocumentAnchorSection().myTopLeft[0];
			if (startX < 30 * window.app.roundedDpiScale + docAncSectionX) {
				pointerIsSyncWithScrollBar = startX + spacer < position.pX;
			}
			else {
				pointerIsSyncWithScrollBar = startX + spacer > position.pX;
			}
		}

		this.sectionProperties.pointerSyncWithHorizontalScrollBar = pointerIsSyncWithScrollBar;
		return pointerIsSyncWithScrollBar;
	}

	public onMouseMove (position: lool.SimplePoint, dragDistance: Array<number>, e: MouseEvent): void {
		this.clearQuickScrollTimeout();

		if (this.sectionProperties.clickScrollVertical && this.containerObject.isDraggingSomething()) {
			if (!this.sectionProperties.previousDragDistance) {
				this.sectionProperties.previousDragDistance = [0, 0];
			}

			this.showVerticalScrollBar();

			var scrollProps: any = this.getVerticalScrollProperties();

			var diffY: number = dragDistance[1] - this.sectionProperties.previousDragDistance[1];
			var actualDistance = scrollProps.ratio * diffY;

			if (this.isMousePointerSyncedWithVerticalScrollBar(scrollProps, position))
				this.scrollVerticalWithOffset(actualDistance);

			this.sectionProperties.previousDragDistance[1] = dragDistance[1];

			e.stopPropagation(); // Don't propagate to map.
			this.stopPropagating(); // Don't propagate to bound sections.
		}
		else if (this.sectionProperties.clickScrollHorizontal && this.containerObject.isDraggingSomething()) {
			if (!this.sectionProperties.previousDragDistance) {
				this.sectionProperties.previousDragDistance = [0, 0];
			}

			this.showHorizontalScrollBar();

			var scrollProps: any = this.getHorizontalScrollProperties();
			var diffX: number = dragDistance[0] - this.sectionProperties.previousDragDistance[0];
			var actualDistance = scrollProps.ratio * diffX;

			if (this.isMousePointerSyncedWithHorizontalScrollBar(scrollProps, position))
				this.scrollHorizontalWithOffset(actualDistance);

			this.sectionProperties.previousDragDistance[0] = dragDistance[0];
			e.stopPropagation(); // Don't propagate to map.
			this.stopPropagating(); // Don't propagate to bound sections.
		}
		else {
			this.isMouseOnScrollBar(position);
		}
	}

	/*
		When user presses the button while the mouse pointer is on the railway of the scroll bar but not on the scroll bar directly,
		we quickly scroll the document to that position.
	*/
	private quickScrollVertical (point: lool.SimplePoint, originalSign?: number): void {
		// Desktop only for now.
		if (!(<any>window).mode.isDesktop())
			return;

		L.DomUtil.addClass(document.documentElement, 'prevent-select');
		var props = this.getVerticalScrollProperties();
		var midY = (props.startY + props.startY + props.scrollSize - this.sectionProperties.scrollBarThickness) * 0.5;

		if (this.stepByStepScrolling) {
			var sign = (point.pY - (props.startY + props.scrollSize)) > 0
				? 1 : ((point.pY - props.startY) < 0 ? -1 : 0);
			var offset = props.verticalScrollStep * sign;

			if (this.sectionProperties.quickScrollVerticalTimer)
				clearTimeout(this.sectionProperties.quickScrollVerticalTimer);
			if (this.sectionProperties.clickScrollVertical)
				this.sectionProperties.quickScrollVerticalTimer = setTimeout(() => {
					if (!originalSign || originalSign === sign) {
						this.quickScrollVertical(point, sign);
					}
				}, this.sectionProperties.stepDuration);
		} else {
			offset = Math.round((point.pY - midY) * props.ratio);
		}

		this.scrollVerticalWithOffset(offset);
	}

	/*
		When user presses the button while the mouse pointer is on the railway of the scroll bar but not on the scroll bar directly,
		we quickly scroll the document to that position.
	*/
	private quickScrollHorizontal (point: lool.SimplePoint, originalSign?: number): void {
		// Desktop only for now.
		if (!(<any>window).mode.isDesktop())
			return;

		L.DomUtil.addClass(document.documentElement, 'prevent-select');
		var props = this.getHorizontalScrollProperties();
		const sizeX = props.scrollSize - this.sectionProperties.scrollBarThickness;
		const docWidth: number = this.map.getPixelBoundsCore().getSize().x;
		const startX = this.isRTL() ? docWidth - props.startX - sizeX : props.startX;
		var midX = startX + sizeX * 0.5;

		if (this.stepByStepScrolling) {
			var sign = (point.pX - (startX + sizeX)) > 0
				? 1 : ((point.pX - startX) < 0 ? -1 : 0);
			var offset = props.horizontalScrollStep * sign;

			if (this.sectionProperties.quickScrollHorizontalTimer)
				clearTimeout(this.sectionProperties.quickScrollHorizontalTimer);
			if (this.sectionProperties.clickScrollHorizontal)
				this.sectionProperties.quickScrollHorizontalTimer = setTimeout(() => {
					if (!originalSign || originalSign === sign) {
						this.quickScrollHorizontal(point, sign);
					}
				}, this.sectionProperties.stepDuration);
		} else {
			offset = Math.round((point.pX - midX) * props.ratio);
		}

		this.scrollHorizontalWithOffset(offset);
	}

	private getLocalYOnVerticalScrollBar (point: lool.SimplePoint): number {
		var props = this.getVerticalScrollProperties();
		return point.pY - props.startY;
	}

	private getLocalXOnHorizontalScrollBar (point: lool.SimplePoint): number {
		var props = this.getHorizontalScrollProperties();
		return point.pX - props.startX;
	}

	private clearQuickScrollTimeout() {
		if (this.sectionProperties.quickScrollVerticalTimer) {
			clearTimeout(this.sectionProperties.quickScrollVerticalTimer);
			this.sectionProperties.quickScrollVerticalTimer = null;
		}
		if (this.sectionProperties.quickScrollHorizontalTimer) {
			clearTimeout(this.sectionProperties.quickScrollHorizontalTimer);
			this.sectionProperties.quickScrollHorizontalTimer = null;
		}
	}

	public onMouseDown (point: lool.SimplePoint, e: MouseEvent): void {
		this.clearQuickScrollTimeout();
		this.onMouseMove(point, null, e);
		this.isMouseOnScrollBar(point);

		const mirrorX = this.isRTL();

		if (this.documentTopLeft[1] >= 0) {
			if ((!mirrorX && point.pX >= this.size[0] - this.sectionProperties.usableThickness)
				|| (mirrorX && point.pY <= this.sectionProperties.usableThickness)) {
				if (point.pY > this.sectionProperties.yOffset) {
					this.sectionProperties.clickScrollVertical = true;
					this.map.scrollingIsHandled = true;
					this.quickScrollVertical(point);
					this.sectionProperties.pointerReCaptureSpacer = this.getLocalYOnVerticalScrollBar(point);
					e.stopPropagation(); // Don't propagate to map.
					this.stopPropagating(); // Don't propagate to bound sections.
				}
				else {
					this.sectionProperties.clickScrollVertical = false;
				}
			}
			else {
				this.sectionProperties.clickScrollVertical = false;
			}
		}

		if (this.documentTopLeft[0] >= 0) {
			if (point.pY >= this.size[1] - this.sectionProperties.usableThickness) {
				if ((!mirrorX && point.pX >= this.sectionProperties.xOffset && point.pX <= this.size[0] - this.sectionProperties.horizontalScrollRightOffset)
					|| (mirrorX && point.pX >= this.sectionProperties.xOffset && point.pX >= this.sectionProperties.horizontalScrollRightOffset)) {
					this.sectionProperties.clickScrollHorizontal = true;
					this.map.scrollingIsHandled = true;
					this.quickScrollHorizontal(point);
					this.sectionProperties.pointerReCaptureSpacer = this.getLocalXOnHorizontalScrollBar(point);
					e.stopPropagation(); // Don't propagate to map.
					this.stopPropagating(); // Don't propagate to bound sections.
				}
				else {
					this.sectionProperties.clickScrollHorizontal = false;
				}
			}
			else {
				this.sectionProperties.clickScrollHorizontal = false;
			}
		}
	}

	public onMouseUp (point: lool.SimplePoint, e: MouseEvent): void {
		L.DomUtil.removeClass(document.documentElement, 'prevent-select');
		this.map.scrollingIsHandled = false;
		this.clearQuickScrollTimeout();

		if (this.sectionProperties.clickScrollVertical) {
			e.stopPropagation(); // Don't propagate to map.
			this.stopPropagating(); // Don't propagate to bound sections.
			this.sectionProperties.clickScrollVertical = false;
			this.sectionProperties.pointerSyncWithVerticalScrollBar = true; // Default.
		}
		else if (this.sectionProperties.clickScrollHorizontal) {
			e.stopPropagation(); // Don't propagate to map.
			this.stopPropagating(); // Don't propagate to bound sections.
			this.sectionProperties.clickScrollHorizontal = false;
			this.sectionProperties.pointerSyncWithHorizontalScrollBar = true; // Default.
		}

		// Unfortunately, dragging outside the map doesn't work for the map element.
		// We will keep this until we remove leaflet.
		else if (L.Map.THIS.mouse && L.Map.THIS.mouse._mouseDown
			&& this.containerObject.targetBoundSectionListContains(L.CSections.Tiles.name)
			&& (<any>window).mode.isDesktop()
			&& this.containerObject.isDraggingSomething()
			&& L.Map.THIS._docLayer._docType === 'spreadsheet') {

			var temp = this.containerObject.getPositionOnMouseUp();
			var tempPos = [temp[0] * app.dpiScale, temp[1] * app.dpiScale];
			var docTopLeft = app.sectionContainer.getDocumentTopLeft();
			tempPos = [tempPos[0] + docTopLeft[0], tempPos[1] + docTopLeft[1]];
			tempPos = [Math.round(tempPos[0] * app.pixelsToTwips), Math.round(tempPos[1] * app.pixelsToTwips)];
			this.onScrollVelocity({ vx: 0, vy: 0 }); // Cancel auto scrolling.
			L.Map.THIS.mouse._mouseDown = false;
			L.Map.THIS._docLayer._postMouseEvent('buttonup', tempPos[0], tempPos[1], 1, 1, 0);
		}

		this.sectionProperties.previousDragDistance = null;
		this.onMouseMove(point, null, e);
	}

	public onClick(point: lool.SimplePoint, e: MouseEvent): void {
		if (this.isAnimating && this.sectionProperties.animatingWheelScrollVertical)
			this.containerObject.stopAnimating();
	}

	private animateScroll(delta: [number, number]): void {
		const lineHeight = this.containerObject.getScrollLineHeight();

		for (let i = 0; i < 2; ++i) {
			if (Math.abs(delta[i]) === 0) continue;

			if ((delta[i] > 0) !== (this.sectionProperties.scrollAnimationAcc[i] > 0)) {
				// Stop animation on scroll change direction
				this.sectionProperties.scrollAnimationVelocity[i] = 0;
				this.sectionProperties.scrollAnimationAcc[i] = 0;
			}

			const sign = delta[i] > 0 ? 1 : -1;
			this.sectionProperties.scrollAnimationDelta[i] =
				lineHeight * ScrollSection.scrollWheelDelta * sign * app.dpiScale;
		}

		if (!this.sectionProperties.animatingScroll) {
			this.sectionProperties.animatingScroll = true;
			this.sectionProperties.lastElapsedTime = 0;
			// We're about to start a duration-less animation, so we need to
			// ensure the animation is reset.
			if (!this.startAnimating({ 'defer': true })) this.resetAnimation();
		}
	}

	public onMouseWheel (point: lool.SimplePoint, delta: Array<number>, e: WheelEvent): void {
		if (e.ctrlKey) return;

		this.map.fire('closepopups'); // close all popups when scrolling

		let hscroll = 0, vscroll = 0;
		if (Math.abs(delta[1]) > Math.abs(delta[0])) {
			if (e.shiftKey)
				hscroll = delta[1];
			else
				vscroll = delta[1];
		} else
			hscroll = delta[0];

		let shouldAnimate = this.sectionProperties.animateWheelScroll
			&& !this.sectionProperties.scrollAnimationDisableTimeout;

		// We don't want to animate in the case of touchpad events. There is no
		// completely browser/OS-agnostic way of determining if a wheel event was
		// generated by a touchpad or a mouse-wheel.
		if (shouldAnimate) {
			// Firefox sends line scroll events for the mouse-wheel and pixel events
			// for the touch-pad. If we receive a non-pixel mousewheel scroll, we know
			// that we can rely on this and disable other heuristics that may cause
			// false-positives.
			if (e.deltaMode !== WheelEvent.DOM_DELTA_PIXEL) {
				this.sectionProperties.scrollQuirks = false;
				// Some touchpads with bad drivers generate mousewheel events. In those cases, the
				// line height will be much smaller and we can treat them as a janky touchpad. Not
				// doing so otherwise makes scrolling difficult to control.
				if (Math.abs((e as any).wheelDeltaY) <= 32 && Math.abs((e as any).wheelDeltaX) <= 32)
					shouldAnimate = false;
			} else if (!this.sectionProperties.scrollQuirks)
				shouldAnimate = false;

			if (e.deltaX !== 0 && e.deltaY !== 0) {
				// It's not a mouse-wheel if both components are non-zero. I suppose it's
				// theoretically possible to scroll in both directions at once with a wheel,
				// but very difficult.
				shouldAnimate = false;
			} else if (this.sectionProperties.scrollQuirks) {
				const nowIsAccurate = performance.now() % 1 !== 0;
				const hasFractionalComponent = (e.deltaX % 1 !== 0) || (e.deltaY % 1 !== 0);
				const deltaMaybeDiscrete = Math.abs((e as any).wheelDelta) % 60 === 0;

				if (hasFractionalComponent && (!nowIsAccurate || !deltaMaybeDiscrete)) {
					// Firefox touchpad deltas always seem to have a fractional
					// component on Linux, but this is also true of wheel events for
					// Chrome on Mac.
					// To distinguish Firefox from Chrome, we can use the fact that
					// Firefox performance.now() is rounded to a whole number and that
					// the wheelDelta on Mac will be discrete.
					shouldAnimate = false;
				} else if (nowIsAccurate && !deltaMaybeDiscrete) {
					// In Chrome, performance.now can (and usually does) have a
					// fractional component. We can use this to single it out, then
					// check if the delta is discrete. This would indicate the event
					// was generated by a mouse-wheel.
					shouldAnimate = false;
				}
			}
		}

		hscroll *= app.dpiScale;
		vscroll *= app.dpiScale;

		if (shouldAnimate)
			this.animateScroll([hscroll, vscroll]);
		else {
			this.sectionProperties.animatingScroll = false;

			if (this.sectionProperties.scrollAnimationDisableTimeout)
				clearTimeout(this.sectionProperties.scrollAnimationDisableTimeout);
			this.sectionProperties.scrollAnimationDisableTimeout =
				setTimeout(() => { this.sectionProperties.scrollAnimationDisableTimeout = null; },
					ScrollSection.scrollDirectTimeoutMs);

			this.sectionProperties.scrollWheelDelta[0] += hscroll;
			this.sectionProperties.scrollWheelDelta[1] += vscroll;

			if (!this.isAnimating) this.startAnimating({});
		}
	}
}

}

L.getNewScrollSection = function (isRTL?: () => boolean) {
	return new lool.ScrollSection(isRTL);
};
