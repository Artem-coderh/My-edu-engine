cd obj
for i in $(find ../ -name '*.c'); do
	echo "CC $i"
	gcc $i -g -c || exit -1
done
echo "LD engine"
gcc *.o -g -o engine || exit -1
echo "RUN test_output"
./engine | magick -size 256x256 -depth 8 gray:- bmp:- | feh -
./engine | magick -size 256x256 -depth 8 gray:- bmp:- > /tmp/debug.bmp
