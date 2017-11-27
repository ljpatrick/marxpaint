#!/bin/sh

BUNDLE=TuxPaint.app
BINARY="$BUNDLE/Contents/MacOS/tuxpaint"
LIBS=`find $BUNDLE/Contents/Resources/lib -type f`
TARGET="$BUNDLE/Contents/lib"


# Sanity check
if [ ! -r "$BINARY" ]; then
	echo "$BINARY: Where is this file?" 1>&2 
	exit 1
fi

if [ ! -d "$BUNDLE" ]; then
	echo "$BUNDLE: Where is this file?" 1>&2 
	exit 1
fi

# Ensure the shared library folder exists
mkdir -p "$TARGET"

# Copy there any shared libraries referenced by the tuxpaint binary, and any
# shared libraries those shared libraries reference, and so on. We do this by
# first copying over any shared libraries referenced by the binary, then
# repeatedly copying over the shared libraries referenced by any libraries in
# the target folder until we don't see any more files appearing. There are
# better ways to do this than copying repeatedly but it works. And you know
# what they say about premature optimization...
dylib="$BINARY $LIBS"
count=0; last=-1
echo "   * Copying Shared Libraries..."
while [ $count -ne $last ]; do
	cp -p `otool -L $dylib | grep '^\t[/]opt[/]local[/]' | sed -e 's/^[[:space:]]*\([^[:space:]]*\)[[:space:]].*/\1/' | sort | uniq` $TARGET
	dylib="$TARGET/*"

	last=$count
	count=`ls -f $dylib | wc -l`
done
echo "     -> Copied" $count "files to $TARGET"

# We just copied over a bunch of shared libraries into a random folder in our
# bundle, but the tuxpaint binary and the shared libraries won't know to look
# in that folder unless we tell them. So we tell them.
echo "   * Fixing Shared Library References..."
for i in "$BINARY" $LIBS $TARGET/*; do
	echo "     -> $i..."
	for j in `otool -L $dylib | grep '^\t[/]opt[/]local[/]' | sed -e 's/^[[:space:]]*\([^[:space:]]*\)[[:space:]].*/\1/'`; do
		n=`echo "$j" | sed 's/^[/]opt[/]local[/]/@executable_path\/..\//'`
		install_name_tool -change "$j" "$n" "$i"
	done
done

