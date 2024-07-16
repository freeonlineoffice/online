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

class CubeTransitionImp extends SimpleTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
		this.prepareTransition();
	}

	public start(): void {
		this.startTransition();
	}
}

function CubeTransition(transitionParameters: TransitionParameters) {
	const slide = new Primitive();
	slide.pushTriangle([0, 0], [1, 0], [0, 1]);
	slide.pushTriangle([1, 0], [0, 1], [1, 1]);

	const aLeavingPrimitives: Primitive[] = [];
	aLeavingPrimitives.push(Primitive.cloneDeep(slide));

	slide.operations.push(
		makeRotateAndScaleDepthByWidth(
			vec3.fromValues(0, 1, 0),
			vec3.fromValues(0, 0, -1),
			90,
			false,
			false,
			0.0,
			1.0,
		),
	);

	const aEnteringPrimitives: Primitive[] = [];
	aEnteringPrimitives.push(slide);

	const aOperations: Operation[] = [];
	aOperations.push(
		makeRotateAndScaleDepthByWidth(
			vec3.fromValues(0, 1, 0),
			vec3.fromValues(0, 0, -1),
			-90,
			false,
			true,
			0.0,
			1.0,
		),
	);

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: aOperations,
	};

	return new CubeTransitionImp(newTransitionParameters);
}

SlideShow.CubeTransition = CubeTransition;
