# COPR runs: make -f .copr/Makefile srpm outdir=<dir>
# then rebuilds the SRPM for every enabled chroot (f43, f44, rawhide, ...).

outdir ?= $(CURDIR)

srpm:
	@command -v rpmbuild >/dev/null || dnf -y install rpm-build
	@command -v git >/dev/null || dnf -y install git
	set -eu; \
	name=yet-another-wireguard-gui; \
	version=$$(rpmspec -q --queryformat '%{VERSION}\n' --srpm yawg.spec | head -1); \
	work=$$(mktemp -d); \
	trap 'rm -rf "$$work"' EXIT; \
	mkdir -p "$$work/SOURCES" "$(outdir)"; \
	git archive --format=tar.gz \
	    --prefix="Yet-Another-Wireguard-Gui-$$version/" \
	    -o "$$work/SOURCES/$$name-$$version.tar.gz" HEAD; \
	rpmbuild -bs yawg.spec \
	    --define "_topdir $$work" \
	    --define "_sourcedir $$work/SOURCES" \
	    --define "_srcrpmdir $(outdir)"

.PHONY: srpm
