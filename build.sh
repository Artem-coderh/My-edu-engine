cd obj
for i in $(find ../ -name '*.c'); do
	echo "CC $i"
	gcc $i -c
done
echo "LD engine"
gcc *.o -o engine
echo "RUN test_output"
./engine | magick -size 256x256 -depth 8 gray:- bmp:- | feh -
