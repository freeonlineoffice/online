/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

declare var SlideShow: any;

class DiamondTransition extends ClippingTransition {
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected getMaskFunction(): string {
		return `
                float getMaskValue(vec2 uv, float time) {
                    float progress = time;

                    vec2 center = vec2(0.5, 0.5);

                    vec2 dist = abs(uv - center);

                    float size = progress * 1.5;

                    float mask = step(dist.x + dist.y, size);

                    return mask;
                }
		`;
	}
}

SlideShow.DiamondTransition = DiamondTransition;
