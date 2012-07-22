#!/usr/bin/env bash
function __FILE__ {
    echo ${BASH_SOURCE[0]}
}
file=$(__FILE__)
dirname=${file%/*}

if cmp /bin/sh /bin/dash 2>/dev/null >/dev/null; then
    echo 'Warning your /bin/sh is dash. Making Ruby might not work!' 1>&2
fi

patchfile=${1:-'1.9.2'}
case $patchfile in
    1.9.3 | head | trunk )
	for file in \
	    000-testit.patch \
	    002-iseq-eval-source-save.patch \
	    003-iseq-field-access.patch \
	    004-iseq-SCRIPT_ISEQS__.patch \
	    005-iseq-top-name.patch 
	do 
	    patch_file=${dirname}/1.9.3/$file
	    echo -- Applying patches in $patch_file ... | tee -a patches_applied.log
	    patch -p1 < $patch_file
	done
	;;
    combined ) 
	file=ruby-trunk-combined.patch
	patch_file=${dirname}/$file
	echo -- Applying patches in $patch_file
	patch -p0 < $patch_file
	;;
    1.9.2 | * )
	file=ruby-1.9.2-combined.patch
	patch_file=${dirname}/$file
	echo -- Applying patches in $patch_file
	patch -p0 < $patch_file
	;;
    esac
