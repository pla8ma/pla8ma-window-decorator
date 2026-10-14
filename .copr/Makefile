# Used by Fedora COPR (SCM source, build method "make_srpm"):
#   make -f .copr/Makefile srpm outdir=<dir> spec=<spec>

top := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)
spec ?= $(top)/pla8ma-window-decorator.spec
outdir ?= $(top)/srpm

name := $(shell sed -n 's/^Name:[[:space:]]*//p' $(spec))
version := $(shell sed -n 's/^Version:[[:space:]]*//p' $(spec))

.PHONY: srpm
srpm:
	command -v git >/dev/null && command -v rpmbuild >/dev/null || dnf -y install git rpm-build
	mkdir -p $(outdir)
	git -c safe.directory=$(top) -C $(top) archive --format=tar.gz --prefix=$(name)-$(version)/ -o $(outdir)/$(name)-$(version).tar.gz HEAD
	rpmbuild -bs $(spec) --define "_sourcedir $(outdir)" --define "_srcrpmdir $(outdir)"
