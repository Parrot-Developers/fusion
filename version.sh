#!/bin/sh

#set -x
# creates an annotated tag with an increased minor number
# the annotation is a release note suitable for a redmine new
# the release_note.py script must be in the path or set in the RELEASE_NOTE
# environment variable

if [ -z "${RELEASE_NOTE}" ];
then
	release_note=$(which release_note.py)
else
	release_note=${RELEASE_NOTE}
fi

set -e

if [ -z "${release_note}" ];
then
	release_note_location="http://mallard/gitweb/?p=release;a=blob_plain;f=release_note.py;hb=HEAD"
	echo "The release_note.py script couldn't be found"
	echo "You can download it with :"
	echo "   wget -O release_note.py '${release_note_location}'"
	echo "   chmod +x release_note.py"
	echo "then put it in the path or use the RELEASE_NOTE variable like :"
	echo "   RELEASE_NOTE=./release_note.py $0"

	exit 1
fi

if [ "$1" = "--help" ]; then
	usage 0
fi

type=$1

# update version numbers where necessary

# find the next tag name
tag_pattern="fusion-[0-9]*\.[0-9]*\.[0-9]*"

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
# version number and the new tag
version=${tag##${project_name}-}
revision=${version##[0-9]*\.[0-9]*\.}
major=${version%%\.[0-9]*}
minor=${version##${major}\.}
minor=${minor%%\.${revision}}

if [ "${type}" = "revision" ]; then
	revision=$((${revision} + 1))
elif [ "${type}" = "minor" ]; then
	minor=$((${minor} + 1))
	revision=0
elif [ "${type}" = "major" ]; then
	major=$((${major} + 1))
	minor=0
	revision=0
fi

new_version=${major}.${minor}.${revision}

new_tag=${project_name}-${new_version}

# then tag, with annotation
message="$(~/workspace/release/release_note.py ${project_path} ${new_tag} ${current_branch})"

echo "${message}" | git tag --annotate --file - ${new_tag}

echo "*** Created new annotated tag ${new_tag} with release note ***"
echo "Once checked, you can push it with :"
echo "   git push ${current_remote} ${new_tag}"
