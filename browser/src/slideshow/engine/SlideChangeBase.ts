/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

abstract class ISlideChangeBase {
	public abstract start(): void;
	public abstract end(): void;
	public abstract perform(nT: number): boolean;
	public abstract getUnderlyingValue(): number;
}

// we use mixin for simulating multiple inheritance
function SlideChangeTemplate<T extends AGConstructor<any>>(BaseType: T) {
	abstract class SlideChangeBase
		extends BaseType
		implements ISlideChangeBase
	{
		private isFinished: boolean;
		private transitionParameters: TransitionParameters;
		protected leavingSlide: WebGLTexture | ImageBitmap;
		protected enteringSlide: WebGLTexture | ImageBitmap;

		constructor(...args: any[]) {
			assert(
				args.length === 1,
				'SlideChangeBase, constructor args length is wrong',
			);

			const transitionParameters: TransitionParameters = args[0];
			super(transitionParameters);
			this.transitionParameters = transitionParameters;
			this.leavingSlide = transitionParameters.current;
			this.enteringSlide = transitionParameters.next;
			this.isFinished = false;
		}

		public abstract start(): void;

		public end(): void {
			if (this.isFinished) return;
			this.isFinished = true;
		}

		public perform(nT: number): boolean {
			if (this.isFinished) return false;
			requestAnimationFrame(this.render.bind(this, nT));
		}

		protected abstract render(nT: number): void;

		public getUnderlyingValue(): number {
			return 0.0;
		}
	}
	return SlideChangeBase;
}

// classes passed to SlideChangeTemplate must have the same number and types of ctor parameters
// expected by SlideChangeBase, so we define the following wrapper class
abstract class TextureRendererCtorForSlideChangeBase extends SimpleTextureRenderer {
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters.context);
	}
}

// SlideChangeGl extends SlideChangeBase, SimpleTextureRenderer
const SlideChangeGl = SlideChangeTemplate(
	TextureRendererCtorForSlideChangeBase,
);
