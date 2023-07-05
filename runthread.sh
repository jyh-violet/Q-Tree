#cmake .
#make clean
#make
result=$1
n=20000000
valueSpan=(30)  # maxValue 2^valueSpan
ratios=(0.75 0.95 1.0) # insert ratio
dataRegionType=(1 3)   #1: random; 3: Zipfian
repeteNum=5
threadNum=(4)
rangeWidth=(100)
zipfPara=(0.99)
  for v in ${valueSpan[*]} ; do
    for t in ${threadNum[*]} ; do
      for data in ${dataRegionType[*]} ; do
        for r in ${ratios[*]} ; do
          for w in ${rangeWidth[*]} ; do
            for para in ${zipfPara[*]} ; do
              rm config.cfg
              echo "TOTAL = $n
dataRegionType = $data
valueSpan = $v
insertRatio = $r
threadnum = $t
rangeWidth = $w
zipfPara = $para" >> config.cfg
	            echo -e "\n" >> $result
              for i in `seq 1 $repeteNum` ; do
                   numactl --physcpubind=0-${t} ./QTree >> $result
                done
              if [[ $data -eq 1 ]]; then
                break
              fi
            done
          done
        done
      done
    done
   done
