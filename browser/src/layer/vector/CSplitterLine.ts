// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/*
 * CSplitterLine is a CRectangle to be used to show the splits when there are freeze-panes.
 */

class CSplitterLine extends CRectangle {

	private isHoriz: boolean = true; // splitter divides X axis (vertical line) ?
	private map: any;
	private origOpacity: number;
	private inactive: boolean;

	constructor(map: any, options: any) {
		super(new lool.Bounds(undefined), options);

		this.fixed = true;
		this.stroke = false;
		this.fill = true;
		this.opacity = 0;

		// Splitters should always be behind other overlays.
		this.zIndex = -Infinity;

		if (options.isHoriz !== undefined)
			this.isHoriz = options.isHoriz;

		this.map = map;

		// preserve original opacity.
		this.origOpacity = this.fillOpacity;

		this.onChange();
	}

	onResize() {
		this.onChange();
	}

	onPositionChange() {
		this.onChange();
	}

	onChange() {
		var newBounds = this.computeBounds();
		this.fillOpacity = this.inactive ? 0 : this.origOpacity;
		this.fillGradient = this.computeShouldFillGradient();
		this.setBounds(newBounds);
	}

	computeShouldFillGradient() {
		const tsManager = this.map._docLayer._painter;
		const splitPos = tsManager.getSplitPos();

		if ((this.isHoriz && !splitPos.x) || (!this.isHoriz && !splitPos.y)
		) {
			return false;
		}

		if (tsManager._inZoomAnim) {
			if (this.isHoriz) {
				return !tsManager._zoomAtDocEdgeX;
			} else {
				return !tsManager._zoomAtDocEdgeY;
			}
		}

		const pixelBounds = this.map.getPixelBounds();

		if (this.isHoriz) {
			return !!pixelBounds.min.x;
		} else {
			return !!pixelBounds.min.y;
		}
	}

	updatePathAllPanes(paintArea?: Bounds) {
		this.fillGradient = this.computeShouldFillGradient();
		super.updatePathAllPanes(paintArea);
	}

	private computeBounds(): lool.Bounds {
		var docLayer = this.map._docLayer;
		var mapSize = this.map.getPixelBoundsCore().getSize();
		mapSize.round();
		var splitPos = docLayer._painter.getSplitPos();
		splitPos.round();

		// For making splitlines appear symmetric w.r.t. headers/grid.
		var thickup = Math.floor(this.thickness / 2) + app.dpiScale;
		var thickdown = Math.ceil(this.thickness / 2);

		// Let the lines be long enough so as to cover the map area at the
		// highest possible zoom level. This makes splitter's
		// zoom animation easier.
		var maxZoom : number = this.map.zoomToFactor(this.map.options.maxZoom);
		var start = new lool.Point(
			this.isHoriz ? splitPos.x - thickup: 0,
			this.isHoriz ? 0 : splitPos.y - thickup);
		var end = new lool.Point(
			this.isHoriz ? splitPos.x + thickdown : mapSize.x * maxZoom,
			this.isHoriz ? mapSize.y * maxZoom : splitPos.y + thickdown)
			._round();

		this.inactive = this.isHoriz ? !splitPos.x : !splitPos.y;
		return new lool.Bounds(start, end);
	}
}
