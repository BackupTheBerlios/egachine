// JavaScript STL
//
// Free Software Licence:
// This software is intended for free use for personal and comercial use.
// No Warranty:
// Because this software is licenced free of charge, there is no warranty
// expressed or implied for any code contained within this file.
// Guyon Roche

// 2006-02-17: Downloaded the code and modified it slightly to fit egachine's
// module handling (Jens Thiele)

(function(STD){


  //
  // RBTree.js
  // implementation of a red black tree
  //
  // Free Software Licence:
  // This software is intended for free use for personal and comercial use.
  // No Warranty:
  // Because this software is licenced free of charge, there is no warranty
  // expressed or implied for any code contained within this file.
  // Guyon Roche



  // logical xor of b1 and b2
  function xor(b1,b2)
  {
    return (b1 && !b2) || (!b1 && b2);
  }

  function RBTree(less)
  {
    this.less = less;
    this.clear();
  }
  RBTree.prototype.clear = function()
    {
      this.root = null;
      this.nSize = 0;
	
      // head and tail are sentinel nodes not stored in the tree
      this.head = {item:"head",prev:null,bSentinel:true};
      this.tail = {item:"tail",tail:null,bSentinel:true};
      this.head.next = this.tail;
      this.tail.prev = this.head;
    }
  RBTree.prototype.insert = function(value,hint)
    {
      var node = new RBTreeNode(value,this);

      // account for size
      if ( !this.nSize++ )
	{
	  this.root = node;
	  node.listInsert(this.head,this.tail);
	  node.parent = null;
	  return node;
	}

      var pos = this.root;
	
      // if hint exists, assume upper bound - 1
      if ( hint && hint.bSentinel )
	{
	  if ( hint === this.head ) hint = hint.next;
	  else if ( hint === this.tail ) hint = hint.prev;
	  else hint = null;
	}
      if ( hint && (hint.tree === this) ) pos = hint;

      pos.insert(node, this.less);

      node.balance();

      // make root node black
      this.root.bBlack = 1;

      return node;
    }

  RBTree.prototype.remove = function(node)
    {
      // remove the node from the tree returning the node that follows.

      // protect and serve
      if ( node.tree !== this ) return this.tail;
	
      // account for size
      if ( --this.nSize == 0 )
	{
	  this.root = null;
	  node.listRemove();
	  return this.tail;
	}
	
      var next = node.next;
		
      // case 1: node has two children or is root
      if ( !node.parent || (node.left.bNode && node.right.bNode) )
	{
	  // find inorder predecessor (or successor) and swap values
	  // after deletion, value order will be preserved
	  var sub = node.prev;
	  if ( !sub )
	    {
	      sub = node.next;
	      next = node;
	    }
	  node.item = sub.item;
	  node = sub;
	}
	
      // node now has at most one child node

      // remove node from linked list
      node.listRemove();

      // p is parent of node
      var p = node.parent;

      var bLeft = (node === p.left);

      // c is child of node
      var c = node.left.bNode ? node.left : node.right;
	
      // move c into place
      c.parent = node.parent;
      if ( bLeft ) p.left = c; else p.right = c;
	
      // node is red
      if ( !node.bBlack )
	{
	  // this is easy, colour c black and return
	  c.bBlack = 1;
	  return next;
	}
	
      // node is black, add to c and fix
      c.bBlack++;
	
      if ( c.bBlack > 1 ) c.fix();

      return next;
    }

  //Returns an iterator to the first element in a set that with a key that is
  //greater than a specified key.
  RBTree.prototype.upper_bound = function(value)
    {
      if ( !this.root ) return this.tail;
      return this.root.upper_bound(value,this.less);
    }

  //Returns an iterator to the first element in a map that with a key value that is 
  //equal to or greater than that of a specified key.
  RBTree.prototype.lower_bound = function(value)
    {
      if ( !this.root ) return this.tail;
      else return this.root.lower_bound(value, this.less);
    }


  function RBTreeNode(value,tree)
  {
    this.item = value;
    this.tree = tree;
    this.bBlack = 0; // default
    this.bNode = true;
	
    // add left and right sentinel nodes
    this.left = {parent:this,bBlack:1,bLeft:true,fix:RBTreeNode.prototype.fix};
    this.right = {parent:this,bBlack:1,bLeft:false,fix:RBTreeNode.prototype.fix};
  }
  RBTreeNode.prototype.rightmost = function()
    {
      if ( this.right.bNode ) return this.right.rightmost();
      else return this;
    }
  RBTreeNode.prototype.leftmost = function()
    {
      if ( this.left.bNode ) return this.left.leftmost();
      else return this;
    }
  RBTreeNode.prototype.listInsert = function(prev,next)
    {	
      this.prev = prev;
      prev.next = this;
	
      this.next = next;
      next.prev = this;
    }
  RBTreeNode.prototype.listRemove = function()
    {
      this.next.prev = this.prev;
      this.prev.next = this.next;
    }
  RBTreeNode.prototype.insertChild = function(prev,next,node)
    {
      node.parent = this;
      node.listInsert(prev,next);
    }
  RBTreeNode.prototype.insert = function(node, less)
    {
      // simple binary insertion
      if ( less.fn(node.item,this.item) )
	{
	  if ( this.left.bNode ) this.left.insert(node,less);
	  else this.insertChild(this.prev,this,this.left = node);
	}
      else
	{
	  if ( this.right.bNode ) this.right.insert(node,less);
	  else this.insertChild(this,this.next,this.right = node);
	}
    }
  RBTreeNode.prototype.balance = function()
    {
      // walk up the tree preserving the red-black rules
	
      // p is parent
      var p = this.parent;

      // case 1: this is root node or parent is black
      if ( !p || (p.bBlack != 0) ) return;
	
      // g is grand parent
      var g = p.parent;
	
      // case 1b: p is root node
      if ( !g ) return;
	
      // record whether we are the left child
      var bLeft = (this === p.left);
	
      // and whether parent is a left child
      var bPLeft = (p === g.left);
	
      // u is uncle
      var u = (bPLeft ? g.right : g.left);
	
      // case 2: parent is red, uncle is black
      if ( u.bBlack != 0 )
	{
	  if ( xor(bLeft,bPLeft) )
	    {
	      // case 2a: parent is different handed to this
	      this.rotate();
	      this.rotate();
	    }
	  else
	    {
	      // case 2b: parent is same handed as this
	      p.rotate();
	    }
	  return;
	}

      // case 3: parent is red, uncle is red
      p.bBlack = 1;
      u.bBlack = 1;
      g.bBlack = 0;
      g.balance();
    }

  RBTreeNode.prototype.rotate = function()
    {
      // p is parent
      var p = this.parent;
	
      // if this is root node, we can't rotate
      if ( !p ) return;
	
      // g is grand parent
      var g = p.parent;
	
      var bLeft = (this === p.left);
	
      if ( bLeft )
	{
	  p.left = this.right;
	  this.right.parent = p;
	  this.right = p;
	}
      else
	{
	  p.right= this.left;
	  this.left.parent = p;
	  this.left = p;
	}
	
      // set parents
      p.parent = this;
      this.parent = g;
      if ( g ) { if ( g.left === p ) g.left = this; else g.right = this; }
      else this.tree.root = this;
	
      // swap colours
      var bBlack = p.bBlack;
      p.bBlack = this.bBlack;
      this.bBlack = bBlack;
    }
  RBTreeNode.prototype.fix = function()
    {
      // fix any colour inconsistancy
	
      // p is parent
      var p = this.parent;
	
      if ( !p )
	{
	  this.bBlack = 1;
	  return;
	}
	
      // s is sibling
      var s = (this === p.left) ? p.right : p.left;
	
      // case 1: black sibling with red child
      if ( s.bBlack && s.bNode )
	{
	  var n = null;
	  if ( !s.left.bBlack ) n = s.left;
	  else if ( !s.right.bBlack ) n = s.right;
	  if ( n )
	    {
	      // restructure
	      this.bBlack = 1;
	      n.bBlack = 1;
	      if ( !xor((s===p.left),(n===s.left)) )
		{
		  // s and n same handed
		  s.rotate();
		}
	      else
		{
		  // s and n different handed
		  n.rotate();
		  n.rotate();
		}
	      return;
	    }		
	}
	
      // case 2: black sibling without (red) children
      if ( s.bBlack )
	{
	  // recolour
	  this.bBlack = 1;
	  s.bBlack = 0;
	  if ( ++p.bBlack > 1 ) p.fix();
	  return;
	}
	
      // case 3: red sibling --> adjustment
      s.rotate();
      this.fix();
    }

  //Returns node of the first element in a set that with a key that is
  //greater than a specified key.
  RBTreeNode.prototype.upper_bound = function(value,less)
    {
      if ( !less.fn(value,this.item) )
	{
	  // value >= this.item
	  if ( this.right.bNode ) return this.right.upper_bound(value,less);
	  else return this.next;
	}
      else
	{
	  // value < this.item
	  if ( this.left.bNode ) return this.left.upper_bound(value,less);
	  else return this;
	}
    }

  //Returns node of the first element in a map that with a key value that is 
  //equal to or greater than that of a specified key.
  RBTreeNode.prototype.lower_bound = function(value,less)
    {
      if ( less.fn(this.item,value) )
	{
	  // value > this.item
	  if ( this.right.bNode ) return this.right.lower_bound(value,less);
	  else return this.next;
	}
      else
	{
	  // value <= this.item
	  if ( this.left.bNode ) return this.left.lower_bound(value,less);
	  else return this;
	}
    }


  STD.map = function(less)
    {
      return new STDMap(less);
    }
  STD.multimap = function(less)
    {
      return new STDMultiMap(less);
    }
  STD.set = function(less)
    {
      return new STDSet(less);
    }
  STD.multiset = function(less)
    {
      return new STDMultiSet(less);
    }
  STD.list = function()
    {
      return new STDList();
    }
  STD.vector = function()
    {
      return new STDVector();
    }
  STD.deque = function()
    {
      return new STDDeque();
    }
  STD.equal_to = 
    {
      // standard equal_to behaviour is to use == operator
      fn:function(a,b)
      {
	return a == b;
      } 
    };
  STD.greater = 
    {
      // standard greater behaviour is to use > operator
      fn:function(a,b)
      {
	return a > b;
      } 
    };
  STD.greater_equal = 
    {
      // standard greater_equal behaviour is to use >= operator
      fn:function(a,b)
      {
	return a >= b;
      } 
    };
  STD.less = 
    {
      // standard less behaviour is to use < operator
      fn:function(a,b)
      {
	return a < b;
      } 
    };
  STD.less_equal = 
    {
      // standard less_equal behaviour is to use <= operator
      fn:function(a,b)
      {
	return a <= b;
      } 
    };
  STD.hash = 
    {
      // standard hash behaviour is to assume value is already an integer
      fn:function(a)
      {
	return a;
      } 
    };
  // create a reference object referencing a value
  STD.ref = function(v)
    {
      return {value:v};
    }
  // create a functor to operate on references
  STD.deRef = function(ftor)
    {
      return { functor:ftor, fn:function(a,b){return this.functor(a.value,b.value);}};
    }

  // useful function to test whether an object is an iterator
  STD.isIterator = function(it,bMyType,bMine)
    {
      // test whether it is an iterator, 
      // options: if it is also a list type, if it is for this list
      if ( !it ) return false;
	
      // assumes all iterators have the following property
      if ( !it.isIterator ) return false;
	
      // also that to be the same type, it has a type property
      if ( bMyType && (it.type != this.type) ) return false;
	
      // and to be contained in this, it has a container property
      if ( bMine && (it.container !== this) ) return false;
	
      return true;
    }

  // "public" since it is required by stlext
  STD.LinkIterator = function(container,node,bForward,type)
  {
    return new STDLinkIterator(container,node,bForward,type);
  }

  //=============================================================================
  // STDLinkIterator
  // An iterator for linked nodes
  function STDLinkIterator(container,node,bForward,type)
  {
    this.container = container;
    this.node = node;
    this.bForward = bForward;
    this.type = type;
	
    this.item = node.item;
  }
  STDLinkIterator.prototype.clone = function()
    {
      return new STDLinkIterator(this.container,this.node,this.bForward,this.type);
    }
  STDLinkIterator.prototype.isIterator = true;
  STDLinkIterator.prototype.incImpl = function(pos, nIncrement, bF)
    {
      if ( bF ) while ( nIncrement-- && pos.next ) pos = pos.next;
      else while ( nIncrement-- && pos.prev ) pos = pos.prev;
      return pos;
    }
  STDLinkIterator.prototype.increment = function(nIncrement)
    {
      if ( arguments.length == 0 ) nIncrement = 1;
      this.node = this.incImpl(this.node, nIncrement, this.bForward);
      this.item = this.node.item;
    }
  STDLinkIterator.prototype.decrement = function(nDecrement)
    {
      if ( arguments.length == 0 ) nDecrement = 1;
      this.node = this.incImpl(this.node, nDecrement, !this.bForward);
      this.item = this.node.item;
    }
  STDLinkIterator.prototype.minus = function(other)
    {
      var n = 0;
      var pos = other.node;
      while ( (pos !== this.node) && !pos.bSentinel )
	{
	  n++;
	  pos = this.incImpl(pos, 1, this.bForward);
	}
      return n;
    }
  STDLinkIterator.prototype.equal_to = function(other)
    {
      return (this.node === other.node);
    }
  STDLinkIterator.prototype.insert = function(value)
    {
      var it = this.container.insert(this,value);
      this.node = it.node;
      this.item = it.item;
    }

  //=============================================================================
  // STDTree
  // basic associative container.
  // wraps RBTree to provide more STL type interface

  function STDTree()
  {
  }
  STDTree.prototype.initialise = function()
    {
      // initialise the tree.
      this.tree = new RBTree(this.less);
    }
  STDTree.prototype.iterator = function(node,bForward)
    {
      if ( arguments.length == 1 ) bForward = true;
      return new STDLinkIterator(this,node,bForward,this.type);	
    }
  STDTree.prototype.isIterator = STD.isIterator;
  STDTree.prototype.createItem = function(key)
    {
      return key;
    }
  STDTree.prototype.begin = function()
    {
      // return iterator at start of collection
      return this.iterator(this.tree.head.next);
    }
  STDTree.prototype.clear = function()
    {
      // remove all items from collection
      this.tree = new RBTree(this.less);
    }
  STDTree.prototype.count = function(key)
    {
      // count the number of items that match key
      var lb = this.lower_bound(key);
      var ub = this.upper_bound(key);
      return ub.minus(lb);
    }
  STDTree.prototype.empty = function()
    {
      // return true if no items contained.
      return this.tree.nSize == 0;
    }
  STDTree.prototype.end = function()
    {
      // return iterator one step beyond the last item
      return this.iterator(this.tree.tail);
    }
  STDTree.prototype.equal_range = function(key)
    {
      // return lower and upper bounds of key
      var lb = this.lower_bound(key);
      var ub = this.upper_bound(key);
      return {first:lb,second:ub};
    }
  STDTree.prototype.eraseImpl = function(nBegin, nEnd)
    {
      while ( nBegin !== nEnd )
	{
	  nBegin = this.tree.remove(nBegin);
	}
      return nBegin;
    }
  STDTree.prototype.erase = function(a,b)
    {
      // erase items from the collection
      if ( arguments.length == 0 ) return;

      if ( !this.isIterator(a,true,true) )
	{
	  // a is a key
	  var value = this.createItem(a);
	  var lb = this.tree.lower_bound(value);
	  var ub = this.tree.upper_bound(value);
	  return this.iterator(this.eraseImpl(lb,ub));
	}
	
      // if b not specified, set it to one past a
      if ( arguments.length == 1 )
	{
	  if ( !a.node.bSentinel ) return this.iterator(this.eraseImpl(a.node, a.node.next));
	  else return this.end();
	}
      else
	{
	  return this.iterator(this.eraseImpl(a.node, b.node));
	}
    }
  STDTree.prototype.find = function(key)
    {
      // return iterator pointing to matching item, or end() if not found
      var item = this.createItem(key);
      var lb = this.tree.lower_bound(item);
      if ( this.less.fn(item,lb.item) ) return this.end();
      else return this.iterator(lb);
    }
  STDTree.prototype.insertReturn = function(item, bIterator, bPair, bSecond)
    {
      // private function to return the correct type of object for 
      // insert(). This can be either the item, an iterator to the
      // item, or a pair containing the item and a boolean value.
      if ( bIterator == undefined ) bIterator = false;
      if ( bPair == undefined ) bPair = false;
      if ( bSecond == undefined ) bSecond = false;
      if ( bIterator && bPair ) return {first:this.iterator(item),second:bSecond};
      else if ( bIterator ) return this.iterator(item);
      else return item;
    }
  STDTree.prototype.insertImpl = function(hint, value, bIterator, bPair)
    {
      // private function to insert a value will an optionally supplied
      // hint position.

      // test hint
      if ( hint && this.less.fn(value,hint.item) ) hint = null;
      else if ( hint && !hint.next.bSentinel && this.less.fn(hint.next.item,value) ) hint = null;
	
      if ( this.bMultiContainer )
	{
	  // always insert into multi container
	  return this.insertReturn(this.tree.insert(value,hint),bIterator);
	}
      else
	{
	  // test whether item or key is unique
	  if ( !hint )
	    {
	      hint = this.tree.lower_bound(value);
	      if ( !hint.bSentinel && !this.less.fn(value,hint.item) )
		{
		  return this.insertReturn(hint, bIterator, bPair, false);
		}
			
	      // by preference, chose the node just before the insertion point
	      if ( !hint.bSentinel && !hint.prev.bSentinel ) hint = hint.prev;
	    }
	  else
	    {
	      if ( !this.less.fn(hint.item,value) )
		{
		  return this.insertReturn(hint, bIterator, bPair, false);
		}
	      if ( !hint.next.bSentinel && !this.less.fn(value,hint.next.item) )
		{
		  return this.insertReturn(hint.next, bIterator, bPair, false);
		}
	    }
	  var node = this.tree.insert(value,hint);
	  return this.insertReturn(node, bIterator, bPair, true);
	}
    }
  STDTree.prototype.insert = function(a,b)
    {
      // insert item(s) into collection
      if ( arguments.length == 0 ) return;
	
      var node = null;
      var hint;
      if ( arguments.length == 1 )
	{
	  // case 1: a is value type
	  return this.insertImpl(null, a, true, true);
	}
	
      if ( this.isIterator(a) && this.isIterator(b) )
	{
	  // case 2: a and b are input iterators. Insert [a,b)
	  hint = null;
	  a = a.clone();
	  while ( !a.equal_to(b) )
	    {
	      hint = this.insertImpl(hint,a.item, false);
	      a.increment();
	    }
	}
	
      if ( this.isIterator(a,true,true) )
	{
	  // case 3: a is a hint. Test and insert,
	  return this.insertImpl(a.node,b,true, false);
	}
    }
  STDTree.prototype.key_comp = function()
    {
      return this.less;
    }
  STDTree.prototype.lower_bound = function(key)
    {
      var node = this.tree.lower_bound(this.createItem(key));
      return this.iterator(node);
    }
  STDTree.prototype.rbegin = function()
    {
      return this.iterator(this.tree.tail.prev,false);
    }
  STDTree.prototype.rend = function()
    {
      return this.iterator(this.tree.head,false);
    }
  STDTree.prototype.size = function()
    {
      return this.tree.nSize;
    }
  STDTree.prototype.swap = function(other)
    {
      var tree = this.tree;
      this.tree = other.tree;
      other.tree = tree;
	
      var less = this.less;
      this.less = other.less;
      other.less = less;
    }
  STDTree.prototype.upper_bound = function(key)
    {
      var node = this.tree.upper_bound(this.createItem(key));
      return this.iterator(node);
    }
  STDTree.prototype.value_comp = function()
    {
      return this.less;
    }

  //=============================================================================
  // STDSet
  // - an associative container
  // - reversible
  // - sorted
  // - unique
  function STDSet(less)
  {
    // initialise map settings
    this.type = "set";
    if ( less ) this.less = less;
    else this.less = STD.less;
    this.bMultiContainer = false;

    // initialise STDTree settings
    this.initialise();
  }

  // inherit behaviour from STDTree
  STDSet.prototype = STDTree.prototype;

  //=============================================================================
  // STDMultiSet
  function STDMultiSet(less)
  {
    // initialise map settings
    this.type = "multiset";
    if ( less ) this.less = less;
    else this.less = STD.less;
    this.bMultiContainer = true;

    // initialise STDTree settings
    this.initialise();
  }

  // inherit behaviour from STDTree
  STDMultiSet.prototype = STDTree.prototype;


  //=============================================================================
  // STDMapLess
  // helper class to STDMap
  function STDMapLess(less)
  {
    if ( less ) this.less = less;
  }
  STDMapLess.prototype.fn = function(a,b)
    {
      return this.less.fn(a.first,b.first);
    }
  STDMapLess.prototype.less = STD.less;

  //=============================================================================
  // STDMap
  // - a pair associative container (key and value)
  // - reversible
  // - sorted
  // - unique
  function STDMap(less)
  {
    // add special map behaviour
    this.createItem = STDMap_createItem;
    this.key_comp = STDMap_key_comp;
    this.value_comp = STDMap_key_comp;
	
    // initialise map settings
    this.type = "map";
    this.less = new STDMapLess(less);
    this.bMultiContainer = false;

    // initialise STDTree settings
    this.initialise();
  }

  // inherit behaviour from STDTree
  STDMap.prototype = STDTree.prototype;

  STDMap_createItem = function(key,value)
    {
      return {first:key,second:value};
    }
  STDMap_key_comp = function()
    {
      return this.less.less;
    }

  //=============================================================================
  // STDMultiMap
  // - a pair associative container (key and value)
  // - reversible
  // - sorted
  function STDMultiMap(less)
  {	
    // add special map behaviour
    this.createItem = STDMap_createItem;
    this.key_comp = STDMap_key_comp;
    this.value_comp = STDMap_key_comp;
	
    // initialise map settings
    this.type = "multimap";
    this.less = new STDMapLess(less);
    this.bMultiContainer = true;

    // initialise STDTree settings
    this.initialise();
  }

  // inherit behaviour from STDTree
  STDMultiMap.prototype = STDTree.prototype;

  //=============================================================================
  // STDList
  // - an ordered container
  // - reversible
  function STDList()
  {
    this.head = {item:"head",prev:null,bSentinel:true};
    this.tail = {item:"tail",tail:null,bSentinel:true};
    this.clear();
  }
  STDList.prototype.type = "list";
  STDList.prototype.createNode = function(item)
    {
      // create a leaf node containing the item
      return {item:item,prev:null,next:null};
    }
  STDList.prototype.insertNode = function(node,prev,next)
    {
      // insert a node between prev and next nodes
      prev.next = node;
      node.prev = prev;
      next.prev = node;
      node.next = next;
      this.nSize++;
    }
  STDList.prototype.removeNode = function(node)
    {
      // remove a node from its current position
      node.prev.next = node.next;
      node.next.prev = node.prev;
      this.nSize--;
    }
  STDList.prototype.iterator = function(node,bForward)
    {
      if ( arguments.length == 1 ) bForward = true;
      return new STDLinkIterator(this,node,bForward,this.type);
    }
  STDList.prototype.isIterator = STD.isIterator;
  STDList.prototype.assign = function(a,b)
    {
      // replace list contents with inputs
      this.clear();
	
      var i = this.head;
      var node;
      if ( this.isIterator(a) && this.isIterator(b) )
	{
	  // insert range [a,b)
	  while ( !a.equal_to(b) )
	    {
	      node = this.createNode(a.item);
	      i.next = node;
	      node.prev = i;
	      i = node;	
	      this.nSize++;
	      a.increment();
	    }
	}
      else
	{
	  // insert value b, a times
	  while ( a > 0 )
	    {
	      node = this.createNode(b);
	      i.next = node;
	      node.prev = i;
	      i = node;	
	      this.nSize++;	
	    }
	}
      this.tail.prev = i;
      i.next = this.tail;
    }
  STDList.prototype.back = function()
    {
      // return the last element (or undefined if there isn't one)
      return (this.nSize > 0) ? this.tail.prev.item : undefined;
    }
  STDList.prototype.begin = function()
    {
      // return iterator at start of list
      return this.iterator(this.head.next);
    }
  STDList.prototype.clear = function()
    {
      // remove all elements from list
      this.head.next = this.tail;
      this.tail.prev = this.head;
      this.nSize = 0;
    }
  STDList.prototype.empty = function()
    {
      return this.nSize == 0;
    }
  STDList.prototype.end = function()
    {
      return this.iterator(this.tail);
    }
  STDList.prototype.erase = function(begin,end)
    {
      var a,b;
      a = begin.node;
      if ( this.isIterator(end) ) b = end.node;
      else b = a.next;

      var prev = a.prev;
      while ( a !== b )
	{
	  this.nSize--;
	  a = a.next;
	}
      prev.next = b;
      b.prev = prev;
    }
  STDList.prototype.front = function()
    {
      // return value at start of list (or undefined if there isn't one)
      return (this.nSize > 0) ? this.head.next.item : undefined;
    }
  STDList.prototype.insert = function(where,a,b)
    {
      var next = where.node;
      var prev = next.prev;
      // insert an item or items into the list
      if ( arguments.length == 2 )
	{
	  node = this.createNode(a);
	  this.insertNode(node,where.node.prev,where.node);
	}
      else
	{
	  if ( this.isIterator(a) && this.isIterator(b) )
	    {
	      while ( !a.equal_to(b) )
		{
		  node = this.createNode(a.item);
		  this.insertNode(node,prev,null);
		  a.increment();
		}
	    }
	  else
	    {
	      while ( a > 0 )
		{
		  node = this.createNode(b);
		  this.insertNode(node,prev,null);
		  a--;
		}
	    }
	  next.prev = prev;
	  prev.next = next;
	}
    }
  STDList.prototype.merge = function(right,less)
    {
      // merge items from another list
      // both lists must already be ordered by less
      this.mergeItems(this.head,this.tail,
		      this.head.next,this.tail,false,
		      right.head.next,right.tail,true,
		      less ? less : STD.less);
	
    }
  STDList.prototype.pop_back = function()
    {
      // removes last element from list
      this.removeNode(this.tail.prev);
    }
  STDList.prototype.pop_front = function()
    {
      // removes first element from list
      this.removeNode(this.head.next);
    }
  STDList.prototype.push_back = function(value)
    {
      // add item to end of list
      var node = this.createNode(value);
      this.insertNode(node,this.tail.prev,this.tail);
    }
  STDList.prototype.push_front = function(value)
    {
      // add item to front of list
      var node = this.createNode(value);
      this.insertNode(node,this.head,this.head.next);
    }
  STDList.prototype.rbegin = function()
    {
      // return reverse iterator at end of list
      return this.iterator(this.tail.prev,false);
    }
  STDList.prototype.remove = function(value)
    {
      // remove all elements that match a value
      var next;
      for ( var i = this.head.next; i != this.tail; i = next )
	{
	  next = i.next;
	  if ( this.equal_to.fn(i.item,value) ) this.removeNode(i);
	}
    }
  STDList.prototype.remove_if = function(pred)
    {
      // remove all elements for which pred.fn(value) is true
      var next;
      for ( var i = this.head.next; i != this.tail; i = next )
	{
	  next = i.next;
	  if ( pred.fn(i.item,value) ) this.removeNode(i);
	}	
    }
  STDList.prototype.rend = function()
    {
      // return reverse iterator at head of list
      return this.iterator(this.head,false);
    }
  STDList.prototype.resize = function(nSize, value)
    {
      while ( nSize < this.nSize ) this.pop_back();
      while ( nSize > this.nSize ) this.push_back(value);
    }
  STDList.prototype.reverse = function()
    {
      // swap items around
      var n = Math.floor(this.nSize / 2);
      var h = this.head.next;
      var t = this.tail.prev;
      while ( n > 0 )
	{
	  var tmp = h.item;
	  h.item = t.item;
	  t.item = tmp;
	  n--;
	}
    }
  STDList.prototype.size = function()
    {
      return this.nSize;
    }
  STDList.prototype.mergeItems = function(wBefore,wAfter,aBegin,aEnd,bCountA,bBegin,bEnd,bCountB,less)
    {
      var i = wBefore;
      var a = aBegin;
      var b = bBegin;
      while ( (a !== aEnd) || (b !== bEnd) )
	{	
	  var bA;
	  if ( a === aEnd ) bA = false;
	  else if ( b === bEnd ) bA = true;
	  else bA = less.fn(a,b);

	  if ( bA )
	    {
	      i.next = a;
	      a.prev = i;
	      i = a;
	      a = a.next;
	      if ( bCountA ) this.nSize++;
	    }
	  else 
	    {
	      i.next = b;
	      b.prev = i;
	      i = b;
	      b = b.next;
	      if ( bCountB ) this.nSize++;
	    }
	}
      i.next = wAfter;
      wAfter.prev = i;
    }
  STDList.prototype.mergeSort = function(begin,end,nCount,less)
    {
      // nothing to do if zero or one items
      if ( nCount < 2 ) return;
	
      // if just two items, swap them
      if ( nCount == 2 )
	{
	  var other = begin.next;
	  if ( less.fn(other, begin) )
	    {
	      var temp = begin.item;
	      begin.item = other.item;
	      other.item = temp;
	    }
	  return;
	}
	
      // else find the mid point and recurse
      var nHalf = Math.floor(nCount/2);
      var mid = begin;
      for ( var i = 0; i < nHalf; i++ ) mid = mid.next;
      this.mergeSort(begin,mid,nHalf,less);
      this.mergeSort(mid,end,nCount-nHalf,less);
	
      // now merge the two
      this.mergeItems(begin.prev,end,begin,mid,false,mid,end,false,less);
    }
  STDList.prototype.sort = function(less)
    {
      this.mergeSort(this.head.next,this.tail,this.nSize,less?less:STD.less);
    }
  STDList.prototype.splice = function(where,right,begin,end)
    {
      switch ( arguments.length )
	{
	  case 2:
	  begin = right.begin();
	  end = right.end();
	  break;
	  case 3:
	  end = right.end();
	  break;
	  case 4:
	  break;
	}
      var lBegin = begin.node;
      var lEnd = end.node;
      var lPrev = lBegin.prev;
	
      var wNext = where.node;
      var wPrev = wNext.prev;
      while ( lBegin !== lEnd )
	{
	  var node = lBegin;
	  lBegin = lBegin.next;
	  this.insertNode(node,wPrev);
	  wPrev = node;
	  right.nSize--;
	}
      wNext.prev = wPrev;
      pPrev.next = wNext;

      lPrev.next = lEnd;
      lEnd.prev = lPrev;
    }
  STDList.prototype.swap = function(right)
    {
      // swap contents with right
      var head = this.head;
      var tail = this.tail;
      var nSize = this.nSize;
      this.head = right.head;
      this.tail = right.tail;
      this.nSize = right.nSize;
      right.head = head;
      right.tail = tail;
      right.nSize = nSize;
    }
  STDList.prototype.unique = function(equal_to)
    {
      if ( equal_to == undefined ) equal_to = STD.equal_to;
      // remove adjacent duplicate items
      for ( var i = this.head.next; i !== this.tail; i = i.next )
	{
	  var iNext = i.next;
	  while ( (iNext!==this.tail) && equal_to.fn(i.item,iNext.item) )
	    {
	      iNext = iNext.next;
	      i.next = iNext;
	      iNext.prev = i;
	    }
	}
    }
  //=============================================================================
  // STDVectorIterator
  function STDVectorIterator(container, nIndex, bForward, type)
  {
    this.container = container;
    this.data = container.data;
    this.bForward = bForward;
    this.type = type;

    this.setPos(nIndex);	
  }
  STDVectorIterator.prototype.clone = function()
    {
      return new STDVectorIterator(this.container,this.getPos(),this.bForward,this.type);
    }
  STDVectorIterator.prototype.setPos = function(nIndex)
    {
      delete this.item;
      this.nIndex = nIndex;
      if ( this.nIndex >= this.container.nEnd ) return;
      if ( this.nIndex < this.container.nBegin ) return;
      this.item = this.data[nIndex];
    }
  STDVectorIterator.prototype.getPos = function()
    {
      return this.nIndex = Math.min(
				    Math.max(this.nIndex,this.container.nBegin-1),
				    this.container.nEnd);
    }
  STDVectorIterator.prototype.increment = function(value)
    {
      if ( arguments.length == 0 ) value = 1;
      this.setPos(this.nIndex + (this.bForward ? value : -value));
    }
  STDVectorIterator.prototype.decrement = function(value)
    {
      if ( arguments.length == 0 ) value = 1;
      this.setPos(this.nIndex + this.bForward ? -value : value);
    }
  STDVectorIterator.prototype.equal_to = function(it)
    {
      return (this.container === it.container) &&
      (this.getPos() == it.getPos());
    }
  STDVectorIterator.prototype.isIterator = true;
  STDVectorIterator.assign = function(value)
    {
      if ( (this.nIndex < this.container.nEnd) &&
	   (this.nIndex >= this.container.nBegin) ) this.data[this.nIndex] = value;
    }
  STDVectorIterator.prototype.minus = function(other)
    {
      return this.getPos() - other.getPos();
    }
  STDLinkIterator.prototype.insert = function(value)
    {
      this.container.insert(this,value);
    }

  //=============================================================================
  // STDVector
  function STDVector()
  {
    this.data = new Array();
    this.nBegin = 0;
    this.nEnd = 0;
    this.type = "vector";
  }
  STDVector.prototype.iterator = function(nPos,bForward)
    {
      bForward = arguments.length == 1 ? true : bForward;
      return new STDVectorIterator(this,nPos,bForward,this.type);
    }
  STDVector.prototype.isIterator = STD.isIterator;
  STDVector.prototype.assign = function(a,b)
    {
      // replace list contents with inputs
      if ( this.isIterator(a) && this.isIterator(b) )
	{
	  this.clear();
	  while ( !a.equal_to(b) )
	    {
	      this.data.push(a.item);
	      a.increment();
	      this.nEnd++;
	    }
	}
      else
	{
	  this.data = new Array(a);
	  this.nBegin = 0;
	  this.nEnd = a;
	  for ( var i = 0; i < a; i++ ) this.data[i] = b;
	}
    }
  STDVector.prototype.at = function(pos)
    {
      return this.data[pos+this.nBegin];
    }
  STDVector.prototype.back = function()
    {
      return this.data[this.nEnd-1];
    }
  STDVector.prototype.begin = function()
    {
      return this.iterator(this.nBegin);
    }
  STDVector.prototype.capacity = function()
    {
      return this.data.length;
    }
  STDVector.prototype.clear = function()
    {
      this.data = new Array();
      this.nBegin = 0;
      this.nEnd = 0;
    }
  STDVector.prototype.empty = function()
    {
      return this.data.length == 0;
    }
  STDVector.prototype.end = function()
    {
      return this.iterator(this.nEnd);
    }
  STDVector.prototype.erase = function(first, last)
    {
      var nFirst = first.getPos();
      if ( nFirst == this.nEnd ) return;
      var nLast;
      if ( arguments.length == 1 ) nLast = nFirst + 1;
      else nLast = last.getPos();
	
      var nCount = nLast - nFirst;
      this.data.splice(nFirst, nCount);
      this.nEnd -= nCount;
    }
  STDVector.prototype.front = function()
    {
      return this.data[this.nBegin];
    }
  STDVector.prototype.insert = function(where,a,b)
    {
      var nWhere = where.getPos();
      if ( arguments.length == 2 )
	{
	  this.data.splice(nWhere,0,a);
	  this.nEnd++;
	  where.item = a;
	  return where;
	}

      var aTail = this.data.splice(nWhere,this.nEnd-nWhere);	
      if ( this.isIterator(a) && this.isIterator(b) )
	{
	  while ( !a.equal_to(b) )
	    {
	      this.data.push(a);
	      this.nEnd++;
	    }
	}
      else
	{
	  for ( var i = 0; i < a; i++ ) this.data.push(b);
	  this.nEnd += a;
	}
      for ( i in aTail ) this.data.push(aTail[i]);
    }

  STDVector.prototype.pop_back = function()
    {
      if ( this.nEnd > this.nBegin ) delete this.data[--this.nEnd];
    }
  STDVector.prototype.push_back = function(value)
    {
      this.data[this.nEnd++] = value;
    }
  STDVector.prototype.rbegin = function()
    {
      return this.iterator(this.nEnd-1, false);
    }
  STDVector.prototype.rend = function()
    {
      return this.iterator(this.nBegin-1,false);
    }
  STDVector.prototype.size = function()
    {
      return this.nEnd - this.nBegin;
    }
  STDVector.prototype.swap = function(right)
    {
      var data = this.data;
      var nBegin = this.nBegin;
      var nEnd = this.nEnd;
	
      this.data = right.data;
      this.nBegin = right.nBegin;
      this.nEnd = right.nEnd;
	
      right.data = data;
      right.nBegin = nBegin;
      right.nEnd = nEnd;
    }

  //=============================================================================
  // STDDeque
  function STDDeque()
  {
    this.data = new Array();
    this.nBegin = 0;
    this.nEnd = 0;
	
    this.type = "deque";
	
    // add a few specialisations...
    this.push_front = STDDeque_push_front;
    this.pop_front = STDDeque_pop_front;
  }

  // inherit methods from STDVector
  STDDeque.prototype = STDVector.prototype;

  STDDeque_push_front = function(value)
    {
      this.data[--this.nBegin] = value;
    }
  STDDeque_pop_front = function()
    {
      if ( this.nEnd > this.nBegin ) delete this.data[this.nBegin++];
    }

 })(this);
