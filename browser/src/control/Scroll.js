/* -*- js-indent-level: 8 -*- */
/*
 * Scroll methods
 */
L.Map.include({
	scrollOffset: function () {
		var center = this.project(this.getCenter());
		var centerOffset = center.subtract(this.getSize().divideBy(2));
		var offset = {};
		offset.x = centerOffset.x < 0 ? 0 : Math.round(centerOffset.x);
		offset.y = Math.round(centerOffset.y);
		return offset;
	},

	scrollTop: function (y) {
		var offset = this.scrollOffset();
		window.app.console.debug('scrollTop: ' + y + ' ' + offset.y + ' ' + (y - offset.y));
		this.panBy(new L.Point(0, y - offset.y));
	},

	scrollLeft: function (x) {
		var offset = this.scrollOffset();
		this.panBy(new L.Point(x - offset.x, 0));
	},
});
