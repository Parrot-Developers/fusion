#!/bin/sh

set -e
#set -x

# update version numbers where necessary

# find the next tag name
tag_pattern="[a-z0-9_]*-[0-9]*\.[0-9]*\.[0-9]*-lib"

tag_list=$(git tag --list ${tag_pattern} | tac)

current_branch=$(git branch | grep \* | sed "s/\* //g")

current_remote=$(git remote)

project_path=$(git remote -v | grep ${current_remote} | head -n1 | sed 's/.*://g' | sed 's/ .*//g')

project_name=$(echo ${project_path} | sed 's#.*/##g' | sed 's/\.git//g' )

# find the last tag on the branch we're on
for tag in ${tag_list};
do
	does_contain=$(git branch --contains ${tag} | sed 's/\*/ /g' | grep ${current_branch})
	if [ -n "${does_contain}" ];
	then
		break;
	fi
done

if [ -z "${does_contain}" ];
then
	# TODO could be replace by an attemps to create the first (X.X.0) tag
	# based on the branch name
	echo "No previous tag found, you have to create one manually"
	exit 1
fi

# we have found the last tag ${tag}, we have to split it now and create the new
# one
tag_prefix=${tag%[0-9]*-lib}
revision=${tag##${project_name}-[0-9]*\.[0-9]*\.}
revision=${revision%%-lib}

revision=$((${revision} + 1))

new_tag=${tag_prefix}${revision}-lib

# update the version, where it is written in source files, makefiles and doc
version_prefix=${tag_prefix##${project_name}-}
major=${version_prefix%%\.[0-9]*\.}
minor=${version_prefix%%\.}
minor=${minor##[0-9]*\.}

# then tag, with annotation
message="$(~/workspace/release/release_note.py ${project_path} ${new_tag} ${current_branch})"

echo "${message}" | git tag --annotate --file - ${new_tag}

echo "*** Created new annotated tag ${new_tag} with release note ***"
