/* -*- js-indent-level: 8 -*- */
/*
 * Simple equirectangular (Plate Carree) projection, used by CRS like EPSG:4326 and Simple.
 */

/* global lool */

L.Projection = {};

L.Projection.LonLat = {
	project: function (latlng) {
		return new lool.Point(latlng.lng, latlng.lat);
	},

	unproject: function (point) {
		return new L.LatLng(point.y, point.x);
	},

	bounds: lool.Bounds.toBounds([-180, -90], [180, 90])
};
