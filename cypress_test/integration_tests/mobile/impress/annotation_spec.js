/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile'], 'Annotation tests.', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('impress/annotation.odp');

		mobileHelper.enableEditingMobile();
	});

	it('Saving comment.', function() {
		mobileHelper.insertComment();

		mobileHelper.selectHamburgerMenuItem(['File', 'Save']);

		helper.reloadDocument(newFilePath);

		mobileHelper.enableEditingMobile();

		mobileHelper.openCommentWizard();

		cy.cGet('#mobile-wizard .wizard-comment-box .lool-annotation-content')
			.should('have.text', 'some text');
	});

	it('Modifying comment.', function() {
		mobileHelper.insertComment();

		mobileHelper.selectAnnotationMenuItem('Modify');

		cy.cGet('.lool-annotation-table').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('have.text', 'some text');
		cy.cGet('#input-modal-input').type('{end}');
		cy.cGet('#input-modal-input').type('modified');
		cy.cGet('#response-ok').click();
		cy.cGet('#toolbar-up #comment_wizard').click();
		cy.cGet('[id^=annotation-content-area-]').should('exist');
		cy.cGet('[id^=annotation-content-area-]').should('have.text', 'some textmodified');
	});

	it('Remove comment.', function() {
		mobileHelper.insertComment();

		cy.cGet('.annotation-marker').should('be.visible');
		cy.cGet('#mobile-wizard .wizard-comment-box .lool-annotation-content').should('have.text', 'some text');

		mobileHelper.selectAnnotationMenuItem('Remove');

		cy.cGet('#mobile-wizard .wizard-comment-box .lool-annotation-content').should('not.exist');
		cy.cGet('.annotation-marker').should('not.exist');
	});

	it('Try to insert empty comment.', function() {
		mobileHelper.openInsertionWizard();

		cy.cGet('body').contains('.menu-entry-with-icon', 'Comment').click();

		cy.cGet('.lool-annotation-table').should('exist');
		cy.cGet('#input-modal-input').should('have.text', '');
		cy.cGet('#response-ok').click();
		cy.cGet('.lool-annotation-content-wrapper.wizard-comment-box').should('not.exist');
		cy.cGet('#mobile-wizard .wizard-comment-box .lool-annotation-content').should('not.exist');
	});
});
