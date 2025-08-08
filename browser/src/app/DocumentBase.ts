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

class DocumentBase {
	public readonly type: string = 'DocumentBase';
	public activeView: ViewLayoutBase;
	protected _fileSize: lool.SimplePoint;

	constructor() {
		this.activeView = new ViewLayoutBase();
		this._fileSize = new lool.SimplePoint(0, 0);
	}

	public get fileSize(): lool.SimplePoint {
		return this._fileSize;
	}

	public set fileSize(value: lool.SimplePoint) {
		this._fileSize = value;
	}
}
