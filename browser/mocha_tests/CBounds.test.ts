/* -*- js-indent-level: 8 -*- */
/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

var assert = require('assert').strict;

describe('Bounds parse() tests', function () {
	describe('Bounds.parse() call with an empty string argument', function () {
		it('should return undefined', function () {
			assert.equal(lool.Bounds.parse(''), undefined);
		});
	});

	describe('Bounds.parse() call with an string argument with 3 numbers', function () {
		it('should return undefined', function () {
			assert.equal(lool.Bounds.parse('10 20 30'), undefined);
		});
	});

	describe('Bounds.parse() call with an string argument with 4 numbers', function () {
		var bounds = lool.Bounds.parse('10 20 30 40');
		it('should return a valid Bounds', function () {
			assert.ok(bounds instanceof lool.Bounds);
			assert.ok(bounds.isValid());
		});

		it('and the Bounds should be correct in position and size', function () {
			assert.ok(
				bounds.equals(
					new lool.Bounds(
						new lool.Point(10, 20),
						new lool.Point(40, 60),
					),
				),
			);
		});
	});

	describe('Bounds constructor call', function () {
		it('correctness check with PointConstructable[] argument', function () {
			var bounds = new lool.Bounds([
				[10, 20],
				{ x: 5, y: 50 },
				[1, 2],
				{ x: -1, y: 7 },
			]);
			var expected = new lool.Bounds(
				new lool.Point(-1, 2),
				new lool.Point(10, 50),
			);
			assert.deepEqual(expected, bounds);
		});
	});
});
