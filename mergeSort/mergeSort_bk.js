function mergeSort(arr){
  //要素の数が０か１だったら配列をそのまま返す
  if(arr.length <= 1){
    return arr;
  }

  //配列の中央を探す
  const mid = Math.floor(arr.length / 2);

  //２つの配列に分ける
  const left = mergeSort(arr.slice(0, mid));
  const right = mergeSort(arr.slice(mid));

  //配列を結合する
  return merge(left, right);
}

function merge(left, right){
  //結合した配列を入れる箱を作る
  const result = [];

  //インデックスは左右の配列ともに０から開始
  let i = 0;
  let j = 0;

  //左右両方の配列に要素が存在する間
  while(i < left.length && j < right.length){
    //左の配列の要素が小さい時
    if(left[i] <= right[j]){
      //左側の要素を配列に加えてインデックスを１進める
      result.push(left[i]);
      i++;
    //右の配列の要素が小さい時
    } else {
      //右側の要素を配列に加えてインデックスを１進める
      result.push(right[j]);
      j++;
    }
  }

  //左側にだけ要素がある時、要素を配列に加えてインデックスを進める
  while(i < left.length){
    result.push(left[i]);
    i++;
  }

  //右側にだけ要素がある時、要素を配列に加えてインデックスを進める
  while(j < right.length){
    result.push(right[j]);
    j++
  }

  return result;
}