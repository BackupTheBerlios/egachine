// JavaScript STL (extension)
//
// Free Software Licence:
// This software is intended for free use for personal and comercial use.
// No Warranty:
// Because this software is licenced free of charge, there is no warranty
// expressed or implied for any code contained within this file.
// Guyon Roche

// 2006-02-17: Downloaded the code and modified it slightly to fit egachine's
// module handling (Jens Thiele)

(function(STDEXT){
  var STD=ejs.ModuleLoader.get("jsstl");

  function STDEXT()
  {
    // namespace class STDEXT
  }

  STDEXT.hash_map = function(hash,less)
    {
      return new STDEXTHashMap(hash,less);
    }
  STDEXT.hash_multimap = function(hash,less)
    {
      return new STDEXTHashMultiMap(hash,less);
    }
  STDEXT.hash_set = function(hash,less)
    {
      return new STDEXTHashSet(hash,less);
    }
  STDEXT.hash_multiset = function(hash,less)
    {
      return new STDEXTHashMultiSet(hash,less);
    }
  STDEXT.hash = 
    {
      // standard hash behaviour is to assume value is already an integer
      fn:function(a)
      {
	return a;
      } 
    };

  //=============================================================================
  // STDEXTHash
  // basic hash based associative container.
  // assumed member list:
  // this.createNode() - create a node from a value with the following members
  //    node.item - the item
  //    node.key - the key used for hashing and comparisons
  //    node.next & node.prev - initially set to null
  // this.less - comparison functor to determine ordering within a hash bucket
  // this.hash - hash functor to determine to which hash bucket a key belongs
  // this.type - string identifying the class
  function STDEXTHash()
  {
  }
  STDEXTHash.prototype.initialise = function(hash,less)
    {
      // assign appropriate hash and less functors
      this.hash = hash ? hash : STDEXT.hash;
      this.less = less ? less : STD.less;

      // create the hash-indexed collection
      this.bucket = new Array();

      // add list sentinels
      this.head = {item:"head",prev:null,bSentinel:true};
      this.tail = {item:"tail",tail:null,bSentinel:true};
      this.head.next = this.tail;
      this.tail.prev = this.head;

      // keep track of count
      this.nCount = 0;
    }
  STDEXTHash.prototype.iterator = function(node,bForward)
    {
      if ( arguments.length == 1 ) bForward = true;
      return new STD.LinkIterator(this,node,bForward,this.type);	
    }
  STDEXTHash.prototype.isIterator = STD.isIterator;
  STDEXTHash.prototype.createNode = function(item)
    {
      // default behaviour; the item is the key
      return {item:item,key:item,next:null,prev:null};
    }
  STDEXTHash.prototype.insertNode = function(node, prev,next)
    {
      // insert a node into the linked list
      node.prev = prev;
      prev.next = node;
	
      node.next = next;
      next.prev = node;
	
      // increment the count
      this.nCount++;
    }
  STDEXTHash.prototype.removeNode = function(node)
    {
      // remove the node from the list
      node.prev.next = node.next;
      node.next.prev = node.prev;

      // decrement the count
      this.nCount--;
    }
  STDEXTHash.prototype.begin = function()
    {
      // return iterator at start of collection
      return this.iterator(this.head.next);
    }
  STDEXTHash.prototype.clear = function()
    {
      // remove all items from collection
      this.bucket = new Array();

      // and empty the list
      this.head.next = this.tail;
      this.tail.prev = this.head;

      // reset the count
      this.nCount = 0;
    }
  STDEXTHash.prototype.count = function(key)
    {
      // count the number of items that match key
      var node = this.lower_bound(key).node;
      var nCount = 0;
      var nHash = this.hash.fn(key);
      while ( !node.bSentinel &&
	      (this.hash.fn(node.key) == nHash) &&
	      !this.less.fn(key,node.key) )
	{
	  nCount++;
	  node = node.next;
	}
      return nCount;
    }
  STDEXTHash.prototype.empty = function()
    {
      // return true if no items contained.
      return this.nCount == 0;
    }
  STDEXTHash.prototype.end = function()
    {
      // return iterator one step beyond the last item
      return this.iterator(this.tail);
    }
  STDEXTHash.prototype.equal_range = function(key)
    {
      // return lower and upper bounds of key
      var lb = this.lower_bound(key);
      var ub = this.upper_bound(key);
      return {first:lb,second:ub};
    }
  STDEXTHash.prototype.eraseImpl = function(node)
    {
      var next = node.next;

      // find the array index
      var h = this.hash.fn(node.key);
	
      // if node is at head of bucket
      if ( this.bucket[h] === node )
	{
	  if ( this.hash.fn(next.key) == h )
	    {
	      // put the next one in this bucket
	      this.bucket[h] = next;
	    }
	  else
	    {
	      // empty the bucket
	      delete this.bucket[h];
	    }
	}
	
      // remove node from linked list
      this.removeNode(node);
	
      // return the next node in the list
      return next;
    }
  STDEXTHash.prototype.erase = function(a,b)
    {
      var iNode, nCount;
	
      // erase items from the collection
      if ( arguments.length == 0 ) return;

      if ( !this.isIterator(a,true,true) )
	{
	  // a is a key
	  iNode = this.lower_bound(a).node;
	  nCount = 0;
	  while ( !this.less.fn(a, iNode.key) )
	    {
	      iNode = this.eraseImpl(iNode);
	      nCount++;
	    }
	  return nCount;
	}
	
      // if b not specified, erase just a
      if ( arguments.length == 1 )
	{
	  if ( !a.node.bSentinel )
	    {
	      return this.iterator(this.eraseImpl(a.node));
	    }
	  else return this.end();
	}
      else
	{
	  iNode = a.node;
	  while ( iNode !== b.node )
	    {
	      iNode = this.eraseImpl(iNode);
	    }
	  return this.iterator(iNode);
	}
    }
  STDEXTHash.prototype.find = function(key)
    {
      var nIndex = this.hash.fn(key);
      var node = this.bucket[nIndex];
      if ( !node ) return this.end();
      while ( !node.bSentinel &&
	      (this.hash.fn(node.key) == nIndex) &&
	      this.less.fn(node.key,key) )
	{
	  node = node.next;
	}
      if ( (this.hash.fn(node.key) == nIndex) &&
	   !this.less.fn(key, node.key) ) return this.iterator(node);
      else return this.end();
    }
  STDEXTHash.prototype.insertImpl = function(value)
    {
      var node = this.createNode(value);
      var nIndex = this.hash.fn(node.key);
      if ( this.bucket[nIndex] )
	{
	  // find the first item in this bin not less than value
	  var pNode = this.bucket[nIndex];
	  while ( (this.hash.fn(pNode.key) == nIndex) &&
		  this.less.fn(pNode.key,node.key) )
	    {
	      pNode = pNode.next;
	    }
		
	  // if not multi container and key exists already, return it
	  if ( !this.bMultiContainer && !this.less.fn(node.key,pNode.key) ) return {first:pNode,second:false};
		
	  // if head of bin, adjust bin
	  if ( pNode === this.bucket[nIndex] ) this.bucket[nIndex] = node;
		
	  // insert in list just before pNode
	  this.insertNode(node, pNode.prev, pNode);
	}
      else
	{
	  this.bucket[nIndex] = node;

	  // insert at tail of list
	  this.insertNode(node, this.tail.prev,this.tail);
	}
      // return new node.
      return {first:node,second:true};
    }
  STDEXTHash.prototype.insertReturn = function(pair)
    {
      // pair: first:node, second:bInserted
      if ( this.bMultiContainer ) return this.iterator(pair.first);
      else return {first:this.iterator(pair.first),second:pair.second};
    }
  STDEXTHash.prototype.insert = function(a,b)
    {
      // insert item(s) into collection
      if ( arguments.length == 0 ) return;
	
      var pair = null;
      var hint;
      if ( arguments.length == 1 )
	{
	  // case 1: a is value type
	  return this.insertReturn(this.insertImpl(a));
	}
	
      if ( this.isIterator(a) && this.isIterator(b) )
	{
	  // case 2: a and b are input iterators. Insert [a,b)
	  node = a.node;
	  while ( node !== b.node )
	    {
	      this.insertImpl(node.item);
	      node = node.next;
	    }
	  return;
	}
	
      if ( this.isIterator(a,true,true) )
	{
	  // case 3: a is a hint. Test and insert
	  if ( !a.node.bSentinel )
	    {
	      node = this.createNode(b);
	      var next = a.node.next;
	      var nHint = this.hash.fn(a.node.key);
	      var nIndex = this.hash.fn(node.key);
	      if ( nHint == nIndex )
		{
		  if ( this.bMultiContainer )
		    {
		      if ( !this.less.fn(node.key,a.node.key) &&
			   (next.bSentinel || 
			    (this.hash.fn(next.key) != nIndex) ||
			    !this.less.fn(next.key,node.key)) )
			{
			  // next is end or in a different hash or not less than b
			  // so insert here
			  this.insertNode(node,a.node,next);
			  return this.iterator(node);
			}
		    }
		  else
		    {
		      if ( this.less.fn(a.node.key, node.key) &&
			   (next.bSentinel || 
			    (this.hash.fn(next.key) != nIndex) ||
			    this.less.fn(node.key,next.key)) )
			{
			  // next is end or in a different hash or not less than b
			  // so insert here
			  this.insertNode(node,a.node,next);
			  return this.iterator(node);
			}
		    }
		}
	    }
	  // if the attempt to use the hint failed, fall back on normal insert
	  return this.iterator(this.insertImpl(b).first);
	}
    }
  STDEXTHash.prototype.key_comp = function()
    {
      return this.less;
    }
  STDEXTHash.prototype.lower_bound = function(key)
    {
      var nIndex = this.hash.fn(key);
      var node = this.bucket[nIndex];
      if ( !node ) return this.end();
      while ( !node.bSentinel &&
	      (this.hash.fn(node.key) == nIndex) &&
	      this.less.fn(node.key,key) )
	{
	  node = node.next;
	}
      return this.iterator(node);
    }
  STDEXTHash.prototype.rbegin = function()
    {
      return this.iterator(this.tail.prev, false);
    }
  STDEXTHash.prototype.rend = function()
    {
      return this.iterator(this.head, false);
    }
  STDEXTHash.prototype.size = function()
    {
      return this.nCount;
    }
  STDEXTHash.prototype.swap = function(other)
    {
      // save the first element for insertion into other.
      var begin = this.head.next;
	
      // insert the contents of other into this
      this.clear();
      this.insert(other.begin(), other.end());
	
      // insert the old contents of this into other. 
      // Note: still terminated by this.end()
      other.clear();
      other.insert(this.iterator(begin), this.end());
    }
  STDEXTHash.prototype.upper_bound = function(key)
    {
      // first element greater than key
      var nIndex = this.hash.fn(key);
      var node = this.bucket[nIndex];
      if ( !node ) return this.end();
      while ( !node.bSentinel &&
	      (this.hash.fn(node.key) == nIndex) &&
	      !this.less.fn(key, node.key) )
	{
	  node = node.next;
	}
      return this.iterator(node);
    }

  //=============================================================================
  // STDEXTHashMap

  function STDEXTHashMap(hash,less)
  {
    // add special map behaviour
    this.createNode = STDEXTHashMap_createNode;
	
    // initialise hash_map settings
    this.type = "hash_map";
    this.bMultiContainer = false;

    // initialise STDEXTHash settings
    this.initialise(hash,less);
  }

  // inherit behaviour from STDEXTHash
  STDEXTHashMap.prototype = STDEXTHash.prototype;

  STDEXTHashMap_createNode = function(item)
    {
      // in a map, the key is item.first
      return {item:item,key:item.first,next:null,prev:null};
    }

  //=============================================================================
  // STDEXTHashMultiMap

  function STDEXTHashMultiMap(hash,less)
  {
    // add special map behaviour
    this.createNode = STDEXTHashMap_createNode;
	
    // initialise hash_multimap settings
    this.type = "hash_multimap";
    this.bMultiContainer = true;

    // initialise STDEXTHash settings
    this.initialise(hash,less);
  }

  // inherit behaviour from STDEXTHash
  STDEXTHashMultiMap.prototype = STDEXTHash.prototype;

  //=============================================================================
  // STDEXTHashSet

  function STDEXTHashSet(hash,less)
  {
    // initialise hash_set settings
    this.type = "hash_set";
    this.bMultiContainer = false;

    // initialise STDEXTHash settings
    this.initialise(hash,less);
  }

  // inherit behaviour from STDEXTHash
  STDEXTHashSet.prototype = STDEXTHash.prototype;

  //=============================================================================
  // STDEXTHashMultiSet

  function STDEXTHashMultiSet(hash,less)
  {
    // initialise map settings
    this.type = "hash_multiset";

    this.bMultiContainer = true;

    // initialise STDEXTHash settings
    this.initialise(hash,less);
  }

  // inherit behaviour from STDEXTHash
  STDEXTHashMultiSet.prototype = STDEXTHash.prototype;
 })(this);
