class Node{
  //識別用のkey、実際のデータvalue、ノードが持つ階層の高さlevelを受け取る
  constructor(key, value, level){
    this.key = key;
    this.value = value;
    //ノードの高さに応じた配列を作る（レベルは０から始まるので、level + 1）
    //.fill(null)で全てnullで初期化
    this.forward = new Array(level + 1).fill(null);
  }
}

class SkipList{
  //引数のデフォルト値を設定しておく
  constructor(maxLevel = 4, p = 0.5){
    //このリストが持つ一番上の階層の数を記憶する
    this.MAX_LEVEL = maxLevel;
    //ノードを挿入するとき、どれくらいの確率で上の階層にもリンクを作るかを決定する係数を記憶（基本はデフォルト値）
    this.p = p;
    //現在リストに存在するノードの中で、一番高いレベルを記録。初期状態では0（最下層のみ）
    this.level= 0;
    //リストの出発点となるヘッダーノードを作成。全ての階層の先頭リンクを保持するため、最大レベルの高さで作成する
    this.header = new Node(null, null, this.MAX_LEVEL);
  }

  //データを挿入するときに、何階層にするかをランダムに決めるメソッド
  _ramdomLevel(){
    //最低でもレベル０（最下層）なので、変数を０で初期化
    let lvl = 0;

    //Math.random()は0以上1未満のランダムな数字を返す。これがthis.p(0.5)未満であり、かつ最大レベルを超えない間
    while(Math.random() < this.p && lvl < this.MAX_LEVEL){
      //レベルを一つずつ上げていく
      lvl++;
    }
    //最終的に決定したノードのレベル（階層の高さ）を返す
    return lvl;
  }

  //指定されたkeyを探すメソッド
  search(key){
    //現在地をヘッダーノードにする
    let current = this.header;

    //リストがある上の階層（this.level）から最下層に向かって１つずつ降りていくループ
    for(let i = this.level; i >= 0; i--){
      //現在地の右隣が存在する、かつ右隣のkeyが探しているkeyよりも小さい間
      while(current.forward[i] !== null && current.forward[i].key < key){
        //現在地を右隣のノードに進める
        current = current.forward[i];

        //右隣のキーが目的のキー以上になるか、右端（null）になるとwhileループを抜け、forループによって一つ下のレベルへ降りる
      }
    }

    //全ての移動が終わったとき、currentは目的のキーの直前のノードを指しているので、右隣のノードへ移動
    current = current.forward[0];

    //移動先のノードが空ではなく、かつキーが探していたキーと一致するとき
    if(current !== null && current.key === key){
      //そのノードのデータを返す
      return current.value;
    }
    //キーを通り過ぎていたり、末尾に達していた時はリストにデータが存在しないためnullを返す
    return null;
  }

  //新しいキーとデータのペアをリストに挿入する
  insert(key, value){
    //スタート地点（現在地）をヘッダーノードの設定
    let current = this.header;
    //新しいノードを差し込む時、各階層のどのノードとどのノードの間に入れるかを知るために、各階層で下に降りる直前にいたノードを記録するための配列を用意する
    let update = new Array(this.MAX_LEVEL + 1).fill(null);

    //一番上の階層から下に降りていく
    for(let i = this.level; i >= 0; i--){
      //右隣のノードが存在して、かつ挿入したいキーより小さい時
      while(current.forward[i] !== null && current.forward[i].key < key){
        //右隣へノードを進める
        current = current.forward[i];
      }
      //右隣へ進めなくなったら、下の階層に降りる直前のノードを記録する
      update[i] = current;
    }
    //全ての移動が終わったら、右隣へノードを進める
    current = current.forward[0];

    //移動先のノードが空ではなく、かつキーが探していたキーと一致するとき
    if(current !== null && current.key === key){
      //新しいデータを上書きする
      current.value = value;
      //処理を終了
      return;
    }

    //探していたキーと一致しなければ、このノードを新規追加。_randomLevel()を使って階層の高さをランダムに決定する
    const rLevel = this._randomLevel();

    //決まった高さが、現在リストにあるどのノードよりも高い階層だった時
    if(rLevel > this.level){
      //未知の階層（this.level + 1 から rLevel まで）をループする
      for(let i = this.level + 1; i <= rLevel; i++){
        //その階層での左隣のノードは先頭のheaderに設定
        update[i] = this.header;
      }
      //リストの最高層の記録を新しい高さに更新
      this.level = rLevel;
    }

    //指定されたキー、データ、階層を持つ新しいノードの実体を生成
    const newNode = new Node(key, value, rLevel);

    //0階層から最高層（rLevel）まで、１回そうずつリンクを繋ぎかえる
    for(let i = 0; i <= rLevel; i++){
      //新しいノードのレベルiの右隣（newNode.forward[i]）に、これまでの左隣のノードが指していた次のノード（update[i].forward[i]）を代入
      newNode.forward[i] = update[i].forward[i];
      //左隣のノードの右隣として、新ノードを指すようにする（矢印の先は、常に本体を指す）
      update[i].forward[i] = newNode;
    }
  }
}

//動作確認
const list = new SkipList();

//データの挿入
list.insert(3, "Apple");
list.insert(6, "Banana");
list.insert(7, "Cherry");
list.insert(9, "Durian");
list.insert(12, "Elderberry");

// 検索のテスト
console.log("キー 6 の検索結果:", list.search(6));  // 出力: Banana
console.log("キー 9 の検索結果:", list.search(9));  // 出力: Durian
console.log("キー 10 の検索結果:", list.search(10)); // 出力: null (存在しない)