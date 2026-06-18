.PHONY: docs-plan docs-book docs-migrate

docs-plan:
	python3 tools/aems/migrate_docs_to_atx100.py

docs-book:
	python3 tools/aems/build_atx100.py
	python3 tools/aems/build_docs_book.py
	python3 tools/aems/migrate_docs_to_atx100.py

docs-migrate:
	python3 tools/aems/migrate_docs_to_atx100.py --apply
