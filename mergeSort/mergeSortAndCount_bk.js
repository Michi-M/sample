function mergeSortAndCount(arr){
  if(arr.length <= 1){
    return{
      sortedArray: arr,
      count: 0
    };
  }

  const mid = Math.floor(arr.length / 2);

  const left = mergeSortAndCount(arr.slice(0, mid));
  const right = mergeSortAndCount(arr.slice(mid));

  const mergeResult = mergeAndCount(left.sortedArray, right.sortedArray);

  return{
    sortedArray: mergeResult.sortedArray,
    count: left.count + right.count + mergeResult.count
  };
}

function mergeAndCount(left, right){
  const result = [];
  let i = 0;
  let j = 0;
  let count = 0;

  while(i < left.length && j < right.length){
    if(left[i] <= right[j]){
      result.push(left[i]);
      i++;
    }else{
      result.push(right[j]);
      j++;
      count += left.length - i;
    }
  }

  while(i < left.length){
    result.push(left[i]);
    i++;
  }

  while(j < right.length){
    result.push(right[j]);
    j++;
  }

  return{
    sortedArray: result,
    count: count
  };
}