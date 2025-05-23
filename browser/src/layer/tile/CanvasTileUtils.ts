/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
declare var L: any;

namespace lool {
	export abstract class CanvasTileUtils {
		private static lastPixel = new Uint8Array(4);

		public static unrle(
			data: Uint8Array,
			width: number,
			height: number,
			output: Uint8Array,
			outputOffset: number = 0,
		): number {
			// Byte bashing fun
			var offset = 0;
			for (var y = 0; y < height; ++y) {
				var rleSize = data[offset] + data[offset + 1] * 256;
				offset += 2;

				var rleMask = offset;
				const rleMaskSizeBytes = 256 / 8;

				offset += rleMaskSizeBytes;

				// It would be rather nice to have real 64bit types [!]
				this.lastPixel.fill(0);
				var lastMask = 0;
				var bitToCheck = 256;
				var rleMaskOffset = rleMask;

				var pixOffset = y * width * 4 + outputOffset;
				var pixSrc = offset;

				for (var x = 0; x < width; ++x) {
					if (bitToCheck > 128) {
						bitToCheck = 1;
						lastMask = data[rleMaskOffset++];
					}
					if (!(lastMask & bitToCheck)) {
						// subarray has a significant overhead on Firefox
						//this.lastPixel.set(data.subarray(pixSrc, pixSrc + 4));
						this.lastPixel[0] = data[pixSrc];
						this.lastPixel[1] = data[pixSrc + 1];
						this.lastPixel[2] = data[pixSrc + 2];
						this.lastPixel[3] = data[pixSrc + 3];
						pixSrc += 4;
					}
					bitToCheck = bitToCheck << 1;

					output.set(this.lastPixel, pixOffset);
					pixOffset += 4;
				}

				offset += rleSize * 4;
			}

			return offset;
		}
	}
} // namespace lool

L.CanvasTileUtils = lool.CanvasTileUtils;
