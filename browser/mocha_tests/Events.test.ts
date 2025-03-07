/// <reference path="./refs/globals.ts"/>
/// <reference path="../src/app/Util.ts"/>
/// <reference path="../src/app/BaseClass.ts"/>
/// <reference path="../src/app/Events.ts" />

var assert = require('assert').strict;

class HandlerData {

	public numCalls: number;
	public event?: EventBaseType;

	constructor() {
		this.numCalls = 0;
		this.event = null;
	}
}

class DerivedEvented extends Evented {

	public first: HandlerData;
	public second: HandlerData;

	constructor() {
		super();
		this.reset();
	}

	public reset(): void {
		this.first = new HandlerData();
		this.second = new HandlerData();
	}

	public handler1(e: EventBaseType): void {
		this.first.numCalls++;
		this.first.event = e;
	}

	public handler2(e: EventBaseType): void {
		this.second.numCalls++;
		this.second.event = e;
	}
}

describe('Evented: Register handler with event names as a string', function () {
	it('no fire() => no handler calls', function () {
		const obj = new DerivedEvented();
		obj.on('e1 e2', obj.handler1, obj);
		obj.on('e3 e4', obj.handler2, obj);

		obj.off('e1 e2', obj.handler1, obj);
		obj.off('e3 e4', obj.handler2, obj);

		assert.equal(0, obj.first.numCalls);
		assert.equal(null, obj.first.event);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});

	it('fire() called after off() must have no effect', function () {
		const obj = new DerivedEvented();
		obj.on('e1 e2', obj.handler1, obj);
		obj.on('e3 e4', obj.handler2, obj);


		obj.off('e1 e2', obj.handler1, obj);
		obj.off('e3 e4', obj.handler2, obj);

		obj.fire('e1');
		obj.fire('e4');

		assert.equal(0, obj.first.numCalls);
		assert.equal(null, obj.first.event);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});

	it('fire() e1 and e4', function () {
		const obj = new DerivedEvented();
		obj.on('e1 e2', obj.handler1, obj);
		obj.on('e3 e4', obj.handler2, obj);

		obj.fire('e1');
		obj.fire('e4');

		obj.off('e1 e2', obj.handler1, obj);
		obj.off('e3 e4', obj.handler2, obj);

		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);

		assert.equal(1, obj.second.numCalls);
		assert.equal('e4', obj.second.event.type);
		assert.equal(obj, obj.second.event.target);
	});

	it('fire() all four events', function () {
		const obj = new DerivedEvented();
		obj.on('e1 e2', obj.handler1, obj);
		obj.on('e3 e4', obj.handler2, obj);

		obj.fire('e1');
		obj.fire('e2');
		obj.fire('e3');
		obj.fire('e4');

		obj.off('e1 e2', obj.handler1, obj);
		obj.off('e3 e4', obj.handler2, obj);

		assert.equal(2, obj.first.numCalls);
		assert.equal('e2', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);

		assert.equal(2, obj.second.numCalls);
		assert.equal('e4', obj.second.event.type);
		assert.equal(obj, obj.second.event.target);
	});

	it('fire() e4 multiple times', function () {
		const obj = new DerivedEvented();
		obj.on('e1 e2', obj.handler1, obj);
		obj.on('e3 e4', obj.handler2, obj);

		obj.fire('e4');
		obj.fire('e4');
		obj.fire('e4');

		obj.off('e1 e2', obj.handler1, obj);
		obj.off('e3 e4', obj.handler2, obj);

		assert.equal(0, obj.first.numCalls);
		assert.equal(null, obj.first.event);

		assert.equal(3, obj.second.numCalls);
		assert.equal('e4', obj.second.event.type);
		assert.equal(obj, obj.second.event.target);
	});

	it('fire() e2 then e1, ensure order of handler calls', function () {
		const obj = new DerivedEvented();
		obj.on('e1 e2', obj.handler1, obj);
		obj.on('e3 e4', obj.handler2, obj);

		obj.fire('e2');
		obj.fire('e1');

		obj.off('e1 e2', obj.handler1, obj);
		obj.off('e3 e4', obj.handler2, obj);

		assert.equal(2, obj.first.numCalls);
		// last called event type is captured.
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});

	it('fire() with data object', function () {
		const obj = new DerivedEvented();
		obj.on('e1', obj.handler1, obj);

		obj.fire('e1', {key1: 42, key2: { inner: 'innerValue'}});

		obj.off('e1', obj.handler1, obj);

		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);
		const eventData = obj.first.event as any;
		assert.equal(42, eventData.key1);
		assert.equal('innerValue', eventData.key2.inner);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});

	it('fire() with no propagate but has parent object', function () {
		const obj = new DerivedEvented();
		const parentObj = new DerivedEvented();

		obj.on('e1', obj.handler1, obj);
		parentObj.on('e1', parentObj.handler1, parentObj);

		// Register parent object.
		obj.addEventParent(parentObj);

		obj.fire('e1', {key1: 42, key2: { inner: 'innerValue'}});

		obj.off('e1', obj.handler1, obj);
		parentObj.off('e1', parentObj.handler1, parentObj);

		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);
		const eventData = obj.first.event as any;
		assert.equal(42, eventData.key1);
		assert.equal('innerValue', eventData.key2.inner);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);

		// No calls should have been made to parent object handlers.
		assert.equal(0, parentObj.first.numCalls);
		assert.equal(null, parentObj.first.event);

		assert.equal(0, parentObj.second.numCalls);
		assert.equal(null, parentObj.second.event);
	});

	it('fire() with propagate and has parent object', function () {
		const obj = new DerivedEvented();
		const parentObj = new DerivedEvented();

		obj.on('e1', obj.handler1, obj);
		parentObj.on('e1', parentObj.handler1, parentObj);

		// Register parent object.
		obj.addEventParent(parentObj);

		obj.fire('e1', {key1: 42, key2: { inner: 'innerValue'}}, true);

		obj.off('e1', obj.handler1, obj);
		parentObj.off('e1', parentObj.handler1, parentObj);

		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		const eventData = obj.first.event as any;
		assert.equal(42, eventData.key1);
		assert.equal('innerValue', eventData.key2.inner);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);

		// Parent object must also have received the call.
		assert.equal(1, parentObj.first.numCalls);
		assert.equal('e1', parentObj.first.event.type);
		const eventData2 = parentObj.first.event as any;
		assert.equal(42, eventData2.key1);
		assert.equal('innerValue', eventData2.key2.inner);
		assert.deepEqual(obj, eventData2.layer);

		assert.equal(0, parentObj.second.numCalls);
		assert.equal(null, parentObj.second.event);
	});

	it('fire() with propagate but with removed parent object', function () {
		const obj = new DerivedEvented();
		const parentObj = new DerivedEvented();

		obj.on('e1', obj.handler1, obj);
		parentObj.on('e1', parentObj.handler1, parentObj);

		// Register parent object.
		obj.addEventParent(parentObj);
		// Deregister the same parent.
		obj.removeEventParent(parentObj);

		obj.fire('e1', {key1: 42, key2: { inner: 'innerValue'}}, true);

		obj.off('e1', obj.handler1, obj);
		parentObj.off('e1', parentObj.handler1, parentObj);

		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);
		const eventData = obj.first.event as any;
		assert.equal(42, eventData.key1);
		assert.equal('innerValue', eventData.key2.inner);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);

		// No calls should have been made to parent object handlers.
		assert.equal(0, parentObj.first.numCalls);
		assert.equal(null, parentObj.first.event);

		assert.equal(0, parentObj.second.numCalls);
		assert.equal(null, parentObj.second.event);
	});

	it('listens() when object is not listening', function () {
		const obj = new DerivedEvented();
		obj.on('e1', obj.handler1, obj);

		assert.equal(false, obj.listens('e2'));

		obj.off('e1', obj.handler1, obj);
	});

	it('listens() when object stopped listening', function () {
		const obj = new DerivedEvented();
		obj.on('e1', obj.handler1, obj);
		obj.off('e1', obj.handler1, obj);

		assert.equal(false, obj.listens('e1'));
	});

	it('listens() when object is listening', function () {
		const obj = new DerivedEvented();
		obj.on('e1', obj.handler1, obj);

		assert.equal(true, obj.listens('e1'));

		obj.off('e1', obj.handler1, obj);
	});

	it('once() without firing', function () {
		const obj = new DerivedEvented();
		obj.once('e1', obj.handler1, obj);

		obj.off('e1', obj.handler1, obj);

		assert.equal(0, obj.first.numCalls);
		assert.equal(null, obj.first.event);

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});

	it('once() with single fire()', function () {
		const obj = new DerivedEvented();
		obj.once('e1', obj.handler1, obj);

		obj.fire('e1', {key1: 42, key2: { inner: 'innerValue'}});

		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);
		const eventData = obj.first.event as any;
		assert.equal(42, eventData.key1);
		assert.equal('innerValue', eventData.key2.inner);
		// off() should have been called automatically
		// so listens() should return false.
		assert.equal(false, obj.listens('e1'));

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});

	it('once() with multiple fire()', function () {
		const obj = new DerivedEvented();
		obj.once('e1', obj.handler1, obj);

		obj.fire('e1', {key1: 42, key2: { inner: 'innerValue'}});
		obj.fire('e1', {key1: 4242, key2: { inner: 'innerValue'}});
		obj.fire('e1', {key1: 424242, key2: { inner: 'innerValue'}});

		// despite the multiple fire() calls, handler must have been
		// called just once.
		assert.equal(1, obj.first.numCalls);
		assert.equal('e1', obj.first.event.type);
		assert.equal(obj, obj.first.event.target);
		const eventData = obj.first.event as any;
		assert.equal(42, eventData.key1);
		assert.equal('innerValue', eventData.key2.inner);
		// off() should have been called automatically
		// so listens() should return false.
		assert.equal(false, obj.listens('e1'));

		assert.equal(0, obj.second.numCalls);
		assert.equal(null, obj.second.event);
	});
});
