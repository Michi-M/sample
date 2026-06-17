class MinHeap{
  constructor(){
    this.heap = [];
  }

  //親子のインデックスを計算する
  getLeftChildIndex(parentIndex){return 2 * parentIndex + 1; }
  getRightChildIndex(parentIndex){return 2 * parentIndex + 2; }
  getParentIndex(childIndex){return Math.floor((childIndex - 1) / 2); }

  //要素を入れ替える
  swap(index1, index2){
    //いったんindex1を記憶しておく
    const temp = this.heap[index1];
    //index2をindex1にいれる
    this.heap[index1] = this.heap[index2];
    //記憶しておいたindex1をindex2に入れることで要素の入れ替え完了
    this.heap[index2] = temp;
  }

  //木の最小値（根の値）をチェック
  peek(){
    return this.heap.length == 0 ? null : this.heap[0];
  }

  //要素を追加する処理
  insert(value){
    //配列の最後に要素を追加
    this.heap.push(value);
    //要素の大きさを比べてルール違反の場合、上へ昇らせる処理をする
    this.heapifyUp();
  }

  heapifyUp(){
    //挿入した要素のインデックスを変数に入れる
    let index = this.heap.length -1;
    //自分が根ノード（インデックス0）に到達していない」かつ「自分の値が、親の値よりも小さい（ルール違反の状態）」である限り、ループを繰り返す
    while(index > 0 && this.heap[index] < this.heap[this.getParentIndex(index)]){
      //自分の位置から親のインデックスを計算
      const parentIndex = this.getParentIndex(index);
      //自分と親の値を入れ替える。これで自分が一つ上の階層に上がる
      this.swap(index, parentIndex);
      //現在の自分の位置を示すindexを、親がいた位置(parentIndex)に更新する
      index = parentIndex;
    }
  }

  //根ノード（最小値）を取り出し、末尾の要素を根に持ってきてから下に向かって沈める処理
  poll(){
    //heapの配列が空なら、取り出すデータがないのでnullを返す
    if(this.heap.length === 0) return null;
    //要素が１つしかない場合は、並び替える必要がないので、そのまま配列からpop()してその値を返す
    if(this.heap.length === 1) return this.heap.pop();

    //根ノードを一時的に記録する
    const minValue = this.heap[0];
    //末尾の要素を根に持ってくる
    this.heap[0] = this.heap.pop();
    //下に沈める処理を行う
    this.heapifyDown();

    //記録してあった本来の根ノード（最小値）を返す
    return minValue;
  }

  heapifyDown(){
    //根ノードからスタートするためにインデックスを0にセットする
    let index = 0;

    //左の子ノードのインデックスの値がヒープの配列の長さより小さい（＝左の子ノードがヒープの配列の範囲内に存在する）場合、ループを回す
    //配列の範囲内に子ノードが存在しない＝子ノードが一つもない＝これ以上下に沈められない
    while(this.getLeftChildIndex(index) < this.heap.length){
      //左の子ノードが小さいと仮定して、そのインデックスを変数に入れる
      let smallerChildIndex = this.getLeftChildIndex(index);
      //比較対象となる右の子ノードのインデックスを変数に入れる
      const rightChildIndex = this.getRightChildIndex(index);

      //右の子ノードが配列内にあって、かつ右の子ノードの値が左の子ノードの値よりも小さいとき
      if(rightChildIndex < this.heap.length && this.heap[rightChildIndex] < this.heap[smallerChildIndex]){
        //右の子ノードのインデックス番号をsmallerChildIndexに入れる
        smallerChildIndex = rightChildIndex;
      }

      //自分（親ノード）の値と左右のうち小さい方の子ノードの値を比べて、親が小さかった時
      if(this.heap[index] <= this.heap[smallerChildIndex]){
        //ルール通りで正しい状態なので、ループを抜ける
        break;
      }

      //自分（親ノード）が子ノードよりも大きかった時、自分と小さい方の子ノードを入れ替える
      this.swap(index, smallerChildIndex);
      //自分が下に移動したため、現在の自分の位置を示すindexを移動先のこのインデックスに更新
      index = smallerChildIndex;
      
      //さらに下の子ノードたちとの比較を行う次のループへ進む
    }
  }
}