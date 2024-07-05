/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class UncoverTransition extends Transition2d {
	private direction: number = 0;
	constructor(
		canvas: HTMLCanvasElement,
		image1: HTMLImageElement,
		image2: HTMLImageElement,
	) {
		super(canvas, image1, image2);
		this.prepareTransition();
		this.animationTime = 1500;
	}

	public renderUniformValue(): void {
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'direction'),
			this.direction,
		);
	}

	public start(direction: number): void {
		this.direction = direction;
		this.startTransition();
	}

	public getVertexShader(): string {
		return `#version 300 es
				in vec4 a_position;
				in vec2 a_texCoord;
				out vec2 v_texCoord;

				void main() {
					gl_Position = a_position;
					v_texCoord = a_texCoord;
				}
				`;
	}

	public getFragmentShader(): string {
		return `#version 300 es
                precision mediump float;

                uniform sampler2D leavingSlideTexture;
                uniform sampler2D enteringSlideTexture;
                uniform float time;
                uniform int direction;

                in vec2 v_texCoord;
                out vec4 outColor;

                void main() {
                    vec2 uv = v_texCoord;
                    float progress = time;

                    vec2 leavingUV = uv;
                    vec2 enteringUV = uv;

                    if (direction == 1) {
                        // Top to bottom
                        leavingUV = uv + vec2(0.0, -progress);
                    } else if (direction == 2) {
                        // Right to left
                        leavingUV = uv + vec2(progress, 0.0);
                    } else if (direction == 3) {
                        // Left to right
                        leavingUV = uv + vec2(-progress, 0.0);
                    } else if (direction == 4) {
                        // Bottom to top
                        leavingUV = uv + vec2(0.0, progress);
                    }
                    else if (direction == 5) {
                        // TODO: Meed to fix this bug, top right to bottom left
                        leavingUV = uv + vec2(progress, -progress);
                    }

                    bool showEntering = false;
                    if (direction == 1) {
                        showEntering = uv.y < progress;
                    } else if (direction == 2) {
                        showEntering = uv.x > 1.0 - progress;
                    } else if (direction == 3) {
                        showEntering = uv.x < progress;
                    } else if (direction == 4) {
                        showEntering = uv.y > 1.0 - progress;
                    } else if (direction == 5) {
                        showEntering = uv.x > 1.0 - progress && uv.y < progress;
                    }

                    if (showEntering) {
                        outColor = texture(enteringSlideTexture, enteringUV);
                    } else {
                        outColor = texture(leavingSlideTexture, leavingUV);
                    }
                }
                `;
	}
}
