/* -*- js-indent-level: 8 -*- */

/*
 * CDarkOverlay is used to render a dark overlay around an OLE object when selected
 */

import Bounds = lool.Bounds;

class CDarkOverlay extends CPathGroup {

	private rectangles: CRectangle[] = [];
	private options: any;

	constructor(pointSet: CPointSet, options: any) {
		super([]);
		this.options = options;
		this.rectangles = this.createRectangles(4);
		this.setPointSet(pointSet);
	}

	private setPointSet(pointSet: CPointSet) {
		var points = pointSet.getPointArray();
		if (!points) {
			for (var i = 0; i < this.rectangles.length; i++) {
				this.rectangles[i].setBounds(
					new lool.Bounds(new lool.Point(0, 0), new lool.Point(0, 1)));
				this.push(this.rectangles[i]);
			}
			return;
		}

		var rectangleBounds = this.invertOleBounds(new lool.Bounds(points[0], points[2]));

		for (var i = 0; i < this.rectangles.length; i++) {
			this.rectangles[i].setBounds(rectangleBounds[i]);
			this.push(this.rectangles[i]);
		}
	}

	private invertOleBounds(oleBounds: lool.Bounds): lool.Bounds[] {
		var rectanglesBounds: lool.Bounds[] = [];

		var minWidth = 0;
		var minHeight = 0;
		var fullWidth = 1000000;
		var fullHeight = 1000000;

		rectanglesBounds.push(new lool.Bounds(new lool.Point(minWidth, minHeight), new lool.Point(fullWidth, oleBounds.min.y)));
		rectanglesBounds.push(new lool.Bounds(new lool.Point(minWidth, oleBounds.min.y), oleBounds.getBottomLeft()));
		rectanglesBounds.push(new lool.Bounds(oleBounds.getTopRight(), new lool.Point(fullWidth, oleBounds.max.y)));
		rectanglesBounds.push(new lool.Bounds(new lool.Point(minWidth, oleBounds.max.y), new lool.Point(fullWidth, fullHeight)));

		return rectanglesBounds;
	}

	private createRectangles(quantity: number): CRectangle[] {
		var rectangles: CRectangle[] = [];
		for (var i = 0; i < quantity; i++) {
			rectangles.push(
				new CRectangle(new lool.Bounds(
					new lool.Point(0, 0), new lool.Point(0, 1)
				), this.options));
		}

		return rectangles;
	}
}
