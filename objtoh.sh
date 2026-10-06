vbody=$(cat teapot.obj | grep "v " | sed -E 's/v ([-0-9.]+) ([-0-9.]+) ([-0-9.]+)/T \{\1, \2, \3\},/' | tr '\n' 'N')
vheader="Vector3_t vertices[$(echo -n $vbody | tr 'N' '\n' | tr 'T' '\t' | wc -l)] = {"
end="};"

echo $vheader
echo -n $vbody | tr 'N' '\n' | tr 'T' '\t'
echo $end

pbody=$(cat teapot.obj | grep "f " | sed -E 's%f ([-0-9.]+)//.* ([-0-9.]+)//.* ([-0-9.]+)//.*%T\1,\nT\2,\nT\3,%' | tr '\n' 'N')
pheader="int polys[$(echo -n $pbody | tr 'N' '\n' | tr 'T' '\t' | wc -l)] = {"
echo $pheader
echo -n $pbody | tr 'N' '\n' | tr 'T' '\t'
echo $end

