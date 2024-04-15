/* global describe it cy beforeEach require afterEach*/

var helper = require('../../common/helper');
var searchHelper = require('../../common/search_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching via search bar.', function() {
	var origTestFileName = 'search_bar.ods';
	var testFileName;

	beforeEach(function() {
		testFileName = helper.beforeAll(origTestFileName, 'calc');
	});

	afterEach(function() {
		helper.afterAll(testFileName, this.currentTest.state);
	});

	it('Search existing word.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		// First cell should be selected
		cy.cGet('input#addressInput').should('have.prop', 'value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});

	it('Search not existing word.', function() {
		cy.cGet('input#addressInput').should('have.prop', 'value', 'A2');

		searchHelper.typeIntoSearchField('q');

		// Should be no new selection
		cy.cGet('input#addressInput').should('have.prop', 'value', 'A2');
	});

	it('Search next / prev instance.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');
		searchHelper.searchNext();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance
		searchHelper.searchNext();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search prev instance
		searchHelper.searchPrev();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});

	it('Search wrap at document end', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance
		searchHelper.searchNext();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance, which is in the beginning of the document.
		searchHelper.searchNext();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});

	it('Cancel search.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		cy.cGet('input#addressInput').should('have.prop', 'value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Cancel search -> selection removed
		searchHelper.cancelSearch();

		cy.cGet('input#search-input').should('be.visible');
	});

});
