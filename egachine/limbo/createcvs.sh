#!/bin/bash
set -e
PACKAGE="egachine"
REP="/var/cvs/jens/${PACKAGE}"
rm -rf ${REP}

cvs -d ${REP} init
TMPDIR="$(mktemp -d)"
cd ${TMPDIR}
cvs -d ${REP} co CVSROOT
cd CVSROOT
cat >> cvswrappers <<-EOF
	*.gif -k 'b'
	*.png -k 'b'
	*.jpg -k 'b'
	*.tif -k 'b'
EOF
cat >> cvsignore <<-EOF
	spidermonkey
	spidermonkey-1.5rc6
EOF
cvs add cvsignore
cvs commit -m "init" cvswrappers cvsignore
cd ~/develop/${PACKAGE}
cvs -d ${REP} import -m "Imported sources" ${PACKAGE} karme start || echo Non successfull return value: $?
cd ${TMPDIR}
cvs -d ${REP} co ${PACKAGE}
echo ${TMPDIR}
find
