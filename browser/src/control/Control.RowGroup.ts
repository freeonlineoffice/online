// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */
/*
 * L.Control.RowGroup
*/

/* global app */

/*
	This file is Calc only. This adds a section for grouped rows in Calc.
	When user selects some rows and groups them using "Data->Group and Outline->Group" menu path, this section is added into
	sections list of CanvasSectionContainer. See _addRemoveGroupSections in file CalcTileLayer.js

	This class is an extended version of "CanvasSectionObject".
*/
namespace lool {

export class RowGroup extends GroupBase {
	name: string = L.CSections.RowGroup.name;
	anchor: any = [[L.CSections.CornerGroup.name, 'bottom', 'top'], 'left'];
	expand: string[] = ['top', 'bottom']; // Expand vertically.
	processingOrder: number = L.CSections.RowGroup.processingOrder;
	drawingOrder: number = L.CSections.RowGroup.drawingOrder;
	zIndex: number = L.CSections.RowGroup.zIndex;

	_sheetGeometry: lool.SheetGeometry;
	_cornerHeaderHeight: number;
	_splitPos: lool.Point;

	constructor() { super(); }

	update(): void {
		if (this.isRemoved) // Prevent calling while deleting the section. It causes errors.
			return;

		this._sheetGeometry = this._map._docLayer.sheetGeometry;
		this._groups = Array(this._sheetGeometry.getRowGroupLevels());

		// Calculate width on the fly.
		this.size[0] = this._computeSectionWidth();

		this._cornerHeaderHeight = this.containerObject.getSectionWithName(L.CSections.CornerHeader.name).size[1];

		this._splitPos = (this._map._docLayer._splitPanesContext as lool.SplitPanesContext).getSplitPos();

		this._collectGroupsData(this._sheetGeometry.getRowGroupsDataInView());
	}

	// This returns the required width for the section.
	_computeSectionWidth(): number {
		return this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * (this._groups.length + 1);
	}

	isGroupHeaderVisible (startY: number, startPos: number): boolean {
		if (startPos > this._splitPos.y) {
			return startY > this._splitPos.y + this._cornerHeaderHeight;
		}
		else {
			return startY >= this._cornerHeaderHeight;
		}
	}

	getEndPosition (endPos: number): number {
		if (endPos <= this._splitPos.y)
			return endPos;
		else {
			return Math.max(endPos + this._cornerHeaderHeight - this.documentTopLeft[1], this._splitPos.y + this._cornerHeaderHeight);
		}
	}

	getRelativeY (docPos: number): number {
		if (docPos < this._splitPos.y)
			return docPos + this._cornerHeaderHeight;
		else
			return Math.max(docPos - this.documentTopLeft[1], this._splitPos.y) + this._cornerHeaderHeight;
	}

	drawGroupControl (group: GroupEntry): void {
		let startX = this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * group.level;
		let startY = this.getRelativeY(group.startPos);
		const endY = this.getEndPosition(group.endPos);
		const strokeColor = this.getColors().strokeColor;

		if (this.isGroupHeaderVisible(startY, group.startPos)) {
			// draw head
			this.context.beginPath();
			this.context.fillStyle = this.backgroundColor;
			this.context.fillRect(this.transformRectX(startX, this._groupHeadSize), startY, this._groupHeadSize, this._groupHeadSize);
			this.context.strokeStyle = strokeColor;
			this.context.lineWidth = 1.0;
			this.context.strokeRect(this.transformRectX(startX + 0.5, this._groupHeadSize), startY + 0.5, this._groupHeadSize, this._groupHeadSize);

			if (!group.hidden) {
				// draw '-'
				this.context.beginPath();
				this.context.moveTo(this.transformX(startX + this._groupHeadSize * 0.25), startY + this._groupHeadSize / 2 + 0.5);
				this.context.lineTo(this.transformX(startX + this._groupHeadSize * 0.75 + app.roundedDpiScale), startY + this._groupHeadSize / 2 + 0.5);
				this.context.stroke();
			}
			else {
				// draw '+'
				this.context.beginPath();
				this.context.moveTo(this.transformX(startX + this._groupHeadSize * 0.25), startY + this._groupHeadSize / 2 + 0.5);
				this.context.lineTo(this.transformX(startX + this._groupHeadSize * 0.75 + app.roundedDpiScale), startY + this._groupHeadSize / 2 + 0.5);

				this.context.stroke();

				this.context.moveTo(this.transformX(startX + this._groupHeadSize * 0.50 + 0.5), startY + this._groupHeadSize * 0.25);
				this.context.lineTo(this.transformX(startX + this._groupHeadSize * 0.50 + 0.5), startY + this._groupHeadSize * 0.75 + app.roundedDpiScale);

				this.context.stroke();
			}
		}

		if (!group.hidden && endY > this._cornerHeaderHeight + this._groupHeadSize && endY > startY) {
			//draw tail
			this.context.beginPath();
			startY += this._groupHeadSize;
			startY = startY >= this._cornerHeaderHeight + this._groupHeadSize ? startY: this._cornerHeaderHeight + this._groupHeadSize;
			startX += this._groupHeadSize * 0.5;
			startX = Math.round(startX);
			startY = Math.round(startY) + 1;
			this.context.strokeStyle = strokeColor;
			this.context.lineWidth = 2.0;
			this.context.moveTo(this.transformX(startX), startY);
			this.context.lineTo(this.transformX(startX), endY - app.roundedDpiScale);
			this.context.stroke();
			this.context.lineTo(Math.round(this.transformX(startX + this._groupHeadSize / 2)), endY - app.roundedDpiScale);
			this.context.stroke();
		}
	}

	drawLevelHeader (level: number): void {
		this.context.beginPath();
		const ctx = this.context;
		const ctrlHeadSize = this._groupHeadSize;
		const levelSpacing = this._levelSpacing;

		const startX = levelSpacing + (ctrlHeadSize + levelSpacing) * level;
		const startY = Math.round((this._cornerHeaderHeight - ctrlHeadSize) * 0.5);

		ctx.strokeStyle = this.getColors().strokeColor;
		ctx.lineWidth = 1.0;
		ctx.strokeRect(this.transformRectX(startX + 0.5, ctrlHeadSize), startY + 0.5, ctrlHeadSize, ctrlHeadSize);
		// draw level number
		ctx.fillStyle = this._textColor;
		ctx.font = this._getFont();
		ctx.textAlign = 'center';
		ctx.textBaseline = 'middle';
		ctx.fillText((level + 1).toString(), this.transformX(startX + (ctrlHeadSize / 2)), startY + (ctrlHeadSize / 2) + 2 * app.dpiScale);
	}

	// Handle user interaction.
	_updateOutlineState (group: Partial<GroupEntry>): void {
		const state = group.hidden ? 'visible' : 'hidden'; // we have to send the new state
		const payload = 'outlinestate type=row' + ' level=' + group.level + ' index=' + group.index + ' state=' + state;
		app.socket.sendMessage(payload);
	}

	// When user clicks somewhere on the section, onMouseClick event is called by CanvasSectionContainer.
	// Clicked point is also given to handler function. This function finds the clicked header.
	findClickedLevel (point: lool.SimplePoint): number {
		if (point.pY < this._cornerHeaderHeight) {
			let index = (this.transformX(point.pX) / this.size[0]) * 100; // Percentage.
			const levelPercentage = (1 / (this._groups.length + 1)) * 100; // There is one more button than the number of levels.
			index = Math.floor(index / levelPercentage);
			return index;
		}
		return -1;
	}

	findClickedGroup (point: lool.SimplePoint): GroupEntry {
		const mirrorX = this.isCalcRTL();
		for (let i = 0; i < this._groups.length; i++) {
			if (this._groups[i]) {
				for (const group in this._groups[i]) {
					if (Object.prototype.hasOwnProperty.call(this._groups[i], group)) {
						const group_ = this._groups[i][group];
						const startX = this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * group_.level;
						const startY = this.getRelativeY(group_.startPos);
						const endX = startX + this._groupHeadSize;
						const endY = startY + this._groupHeadSize;
						if (group_.level == 0 && this.isPointInRect(point, startX, startY, endX, endY, mirrorX))
							return group_;
						else if (this._isPreviousGroupVisible(group_.level, group_.startPos, group_.endPos, group_.hidden) && this.isPointInRect(point, startX, startY, endX, endY, mirrorX)) {
							return group_;
						}
					}
				}
			}
		}
		return null;
	}

	getTailsGroupRect (group: GroupEntry): number[] {
		const startX = this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * group.level;
		const startY = this.getRelativeY(group.startPos);
		const endX = startX + this._groupHeadSize; // Let's use this as thikcness. User doesn't have to double click on a pixel:)
		const endY = group.endPos + this._cornerHeaderHeight - this.documentTopLeft[1];
		return [startX, endX, startY, endY];
	}

	onRemove(): void {
		this.isRemoved = true;
		this.containerObject.getSectionWithName(L.CSections.RowHeader.name).position[0] = 0;
		this.containerObject.getSectionWithName(L.CSections.CornerHeader.name).position[0] = 0;
	}
}

}

L.Control.RowGroup = lool.RowGroup;

L.control.rowGroup = function () {
	return new L.Control.RowGroup();
};
