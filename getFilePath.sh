#!/bin/bash
#folder="./test"

echo "paths" > path.txt
function readfile ()
{
#这里`为esc下面的按键符号
  for file in `ls $1`
  do
#这里的-d表示是一个directory，即目录/子文件夹
    if [ -d $1"/"$file ]
    then
#如果子文件夹则递归
      readfile $1"/"$file
    else
#否则就能够读取该文件的地址
      echo $1"/"$file
      echo $1"/"$file >> path.txt
   fi
  done
}
#函数定义结束，这里用来运行函数
folder="src/main/cpp"
readfile $folder