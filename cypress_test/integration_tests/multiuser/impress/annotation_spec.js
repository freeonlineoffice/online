/* global describe it cy require afterEach beforeEach Cypress */

var desktopHelper = require('../../common/desktop_helper');
var helper = require('../../common/helper');
var { insertMultipleComment, setupUIforCommentInsert, createComment } = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'Multiuser Annotation Tests', function() {
	var origTestFileName = 'comment_switching.odp';
	var testFileName;

	beforeEach(function() {

		testFileName = helper.beforeAll(origTestFileName, 'impress', undefined, true);
		cy.viewport(2600, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebarIfVisible();
		}
		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50');

		cy.cSetActiveFrame('#iframe2');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebarIfVisible();
		}
		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50');
	});

	afterEach(function() {
		helper.afterAll(testFileName, this.currentTest.state);
	});

	it('Insert', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
	});

	it('Modify', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('{home}');
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#annotation-content-area-1').should('contain','some other text, some text0');
		cy.cGet('.leaflet-marker-icon').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#annotation-content-area-1').should('contain','some other text, some text0');
		cy.cGet('.leaflet-marker-icon').should('exist');
	});

	it('Remove',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Remove').click();
		cy.cGet('.leaflet-marker-icon').should('not.exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('not.exist');
	});

	it('Reply',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#annotation-reply-1').click();
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});
});

describe(['tagmultiuser'], 'Multiuser Collapsed Annotation Tests', function() {
	var testFileName = 'comment_switching.odp';

	beforeEach(function() {
		helper.beforeAll(testFileName, 'impress', undefined, true);
		cy.viewport(2400, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebarIfVisible();
		}
		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50');

		cy.cSetActiveFrame('#iframe2');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebarIfVisible();
		}
		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50');
	});

	afterEach(function() {
		helper.afterAll(testFileName, this.currentTest.state);
	});

	it('Insert', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
	});

	it('Modify', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('#annotation-content-area-1').should('contain','some text0');
		cy.cGet('.avatar-img').click();
		cy.cGet('.lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('{home}');
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#annotation-content-area-1').should('contain','some other text, some text0');
		cy.cGet('.leaflet-marker-icon').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#annotation-content-area-1').should('contain','some other text, some text0');
		cy.cGet('.leaflet-marker-icon').should('exist');
	});

	it('Remove',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.avatar-img').click();
		cy.cGet('.lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Remove').click();
		cy.cGet('.leaflet-marker-icon').should('not.exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('not.exist');
	});

	it('Reply',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('contain','some text');
		cy.cGet('.avatar-img').click();
		cy.cGet('.lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#annotation-reply-1').click();
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});
});

describe(['tagmultiuser'], 'Multiuser Annotation Autosave Tests', function() {
	var origTestFileName = 'comment_switching.odp';
	var testFileName;

	beforeEach(function() {
		testFileName = helper.beforeAll(origTestFileName, 'impress', undefined, true);
		cy.viewport(2600, 800);
		desktopHelper.switchUIToNotebookbar();

		cy.cSetActiveFrame('#iframe1');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebarIfVisible();
		}
		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50');

		cy.cSetActiveFrame('#iframe2');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.hideSidebarIfVisible();
		}
		cy.cGet('#options-modify-page').click();
		desktopHelper.selectZoomLevel('50');
	});

	afterEach(function() {
		helper.afterAll(testFileName, this.currentTest.state);
	});

	it('Insert autosave', function() {
		cy.cSetActiveFrame('#iframe1');
		setupUIforCommentInsert('impress');
		createComment('impress', 'Test Comment', false, '#insert-insert-annotation-button');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','Test Comment');
	});

	it('Insert autosave save', function() {
		cy.cSetActiveFrame('#iframe1');
		setupUIforCommentInsert('impress');
		createComment('impress', 'Test Comment', false, '#insert-insert-annotation-button');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.not.visible');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','Test Comment');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','Test Comment');
	});

	it('Insert autosave cancel', function() {
		cy.cSetActiveFrame('#iframe1');
		setupUIforCommentInsert('impress');
		createComment('impress', 'Test Comment', false, '#insert-insert-annotation-button');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('.lool-annotation-autosavelabel').should('not.exist');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('not.exist');
		cy.cGet('.leaflet-marker-icon').should('not.exist');
		cy.cGet('.lool-annotation-content > div').should('not.exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('not.exist');
		cy.cGet('.lool-annotation-content > div').should('not.exist');
	});

	it('Modify autosave', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('{home}');
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','some other text, some text0');
	});

	it('Modify autosave save', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('{home}');
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('#annotation-content-area-1').should('have.text','some other text, some text0');
		cy.cGet('.leaflet-marker-icon').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','some other text, some text0');
	});

	it('Modify autosave cancel', function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Modify').click();
		cy.cGet('#annotation-modify-textarea-1').type('{home}');
		cy.cGet('#annotation-modify-textarea-1').type('some other text, ');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('.lool-annotation-edit.modify-annotation').should('be.visible');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('#annotation-content-area-1').should('have.text','some text0');
		cy.cGet('.leaflet-marker-icon').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
	});

	it('Reply autosave',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('#annotation-modify-textarea-1').should('be.visible');
		cy.cGet('#annotation-modify-textarea-1').should('include.text', 'some text0');
		cy.cGet('#annotation-modify-textarea-1').should('include.text', 'some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});

	it('Reply autosave save',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('#annotation-modify-textarea-1').should('be.visible');
		cy.cGet('#annotation-modify-textarea-1').should('include.text', 'some text0');
		cy.cGet('#annotation-modify-textarea-1').should('include.text', 'some reply text');
		cy.cGet('#annotation-save-1').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('include.text','some text0');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('include.text','some reply text');
	});

	it('Reply autosave cancel',function() {
		cy.cSetActiveFrame('#iframe1');
		insertMultipleComment('impress', 1, false, '#insert-insert-annotation-button');
		cy.cGet('.leaflet-marker-icon').should('exist');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
		cy.cGet('.lool-annotation-content-wrapper:visible .lool-annotation-menu').click();
		cy.cGet('body').contains('.context-menu-item','Reply').click();
		cy.cGet('#annotation-reply-textarea-1').type('some reply text');
		cy.cGet('#map').focus();
		cy.cGet('.lool-annotation-autosavelabel').should('be.visible');
		cy.cGet('#annotation-modify-textarea-1').should('be.visible');
		cy.cGet('#annotation-modify-textarea-1').should('include.text', 'some text0');
		cy.cGet('#annotation-modify-textarea-1').should('include.text', 'some reply text');
		cy.cGet('#annotation-cancel-1').click();
		cy.cGet('.lool-annotation-autosavelabel').should('be.not.visible');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('.lool-annotation-edit.reply-annotation').should('be.not.visible');
		cy.cGet('.lool-annotation-content > div').should('have.text','some text0');
	});
});
