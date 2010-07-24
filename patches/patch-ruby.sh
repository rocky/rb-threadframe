#!/usr/bin/env bash
function __FILE__ {
    echo ${BASH_SOURCE[0]}
}
file=$(__FILE__)
dirname=${file%/*}

patchfile=${1:-'head'}
case $patchfile in
    1.9.3 | head | trunk )
	for file in \
	    00-extern-access.patch  \
	    01-get-sourceline.patch \
	    02-disasm-insns.patch  \
	    03-iseq-access.patch   \
	    04-iseq-create.patch   \
	    05-C-argc.patch        \
	    06-brkpt.patch         \
	    07-raise-msg.patch     \
	    08-trace_func.patch
	do 
	    patch_file=${dirname}/trunk/$file
	    echo -- Applying patches in $patch_file ... | tee -a patches_applied.log
	    patch -p0 < $patch_file
	done
	;;
    combined ) 
	file=ruby-trunk-combined.patch
	patch_file=${dirname}/$file
	echo -- Applying patches in $patch_file
	patch -p0 < $patch_file
	;;
    1.9.2 | rc2  | * )
	file=ruby-1.9.2-combined.patch
	patch_file=${dirname}/$file
	echo -- Applying patches in $patch_file
	patch -p0 < $patch_file
	;;
    esac
