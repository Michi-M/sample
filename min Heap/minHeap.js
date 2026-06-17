class MinHeap{
  constructor(){
    this.heap = [];
  }

  getLeftChildIndex(parentIndex){return 2 * parentIndex + 1;}
  getRightChildIndex(parentIndex){return 2 * parentIndex + 2;}
  getParentIndex(childIndex){return Math.floor((childIndex - 1) / 2);}

  swap(index1, index2){
    const temp = this.heap[index1];
    this.heap[index1] = this.heap[index2];
    this.heap[index2] = temp;
  }

  peek(){
    return this.heap.length === 0 ? null : this.heap[0];
  }

  insert(value){
    this.heap.push(value);
    this.heapifyUp();
  }

  heapifyUp(){
    let index = this.heap.length - 1;
    while(index > 0 && this.heap[index] < this.heap[this.getParentIndex(index)]){
      const parentIndex = this.getParentIndex(index);
      this.swap(index, parentIndex);
      index = parentIndex;
    }
  }

  poll(){
    if(this.heap.length === 0) return null;
    if(this.heap.length === 1) return this.heap.pop();

    const minValue = this.heap[0];
    this.heap[0] = this.heap.pop();
    this.heapifyDown();

    return minValue;
  }

  heapifyDown(){
    let index = 0;

    while(this.getLeftChildIndex(index) < this.heap.length){
      let smallerChildIndex = this.getLeftChildIndex(index);
      const rightChildIndex = this.getRightChildIndex(index);

      if(rightChildIndex < this.heap.length && this.heap[rightChildIndex] < this.heap[smallerChildIndex]){
        smallerChildIndex = rightChildIndex
      }

      if(this.heap[index] <= this.heap[smallerChildIndex]){
        break;
      }

      this.swap(index, smallerChildIndex);
      index = smallerChildIndex;
    }
  }
}