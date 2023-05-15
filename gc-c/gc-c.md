#### 垃圾回收简易版 （C）

##### 运行过程

- 由 VM 负责创建对象，并将对象压入维护对象指针的栈中；用不到的对象（需要被回收）出栈；
- VM 维护列表头，用以记录经由 VM 创建的所有对象（包括已经栈中存放的）；
- 触发 gc 时候，通过遍历列表，标记存放在栈中的对象（不回收），然后调用 sweep 执行回收内存的操作！

##### 相关对象

对象类型：

```c
typedef enum
{
  OBJ_INT,
  OBJ_PAIR
} ObjectType;

typedef struct sObject
{
  ObjectType type;		// 对象类型
  unsigned char marked;	// 记录是否需要被回收
  struct sObject *next; // 堆上下一个对象
  union
  {
    int value;					// INT 对象
    struct						// PAIR 对象
    {	
      struct sObject *first;
      struct sObject *second;
    };
  };
} Object;
```

虚拟机：

```c
#define STACK_MAX 256      /* 栈容量 */
typedef struct
{
  Object *stack[STACK_MAX]; // 存放不需要回收的对象指针
  int stackSize;			// 栈中元素数量

  Object *firstObject;		// 堆中对象的头指针

  int numObjects;			// 堆中对象的数量
  int maxObjects;			// 触发gc 的阈值
} VM;
```

> 注意区分 stackSize 和 numObjects 



##### 重要函数

```c
VM *newVM();                                // 创建虚拟机
void freeVM(VM *vm);                        // 释放虚拟机
Object *newObject(VM *vm, ObjectType type); // 经由虚拟机创建对象
void push(VM *vm, Object *value);           // 对象放入栈中
void pushInt(VM *vm, int intValue);         // 创建 Int 类型对象并入栈
Object *pushPair(VM *vm);                   // 创建 Pair 类型对象并入栈
Object *pop(VM *vm);                        // 对象出栈
void mark(Object *object);                  // 标记对象-不回收
void gc(VM *vm);                            // 触发垃圾回收
void markAll(VM *vm);                       // 标记所有在栈中的对象
void sweep(VM *vm);                         // 回收所有未标记的对象
```



##### 创建虚拟机

```c
#define INIT_OBJ_NUM_MAX 8 /* 触发GC 回收 */

VM *newVM()
{
  VM *vm = malloc(sizeof(VM));
  vm->stackSize = 0;
  vm->firstObject = NULL;
  vm->numObjects = 0;
  vm->maxObjects = INIT_OBJ_NUM_MAX;
  return vm;
}
```



##### 创建对象

vm 创建对象：

```c
Object *newObject(VM *vm, ObjectType type)
{
  if (vm->numObjects == vm->maxObjects)
  {
    gc(vm);
  }

  Object *object = malloc(sizeof(Object));
  object->type = type;
  object->next = vm->firstObject;
  object->marked = 0;

  vm->firstObject = object;
  vm->numObjects++;
  return object;
}
```

INT：

```c
void pushInt(VM *vm, int intValue)
{
  Object *object = newObject(vm, OBJ_INT);
  object->value = intValue;
  push(vm, object);
}
```

PAIR：

```c
Object *pushPair(VM *vm)
{
  Object *object = newObject(vm, OBJ_PAIR);
  object->second = pop(vm);
  object->first = pop(vm);

  push(vm, object);
  return object;
}
```

##### 对象入栈出栈

入栈：

```c
void push(VM *vm, Object *value)
{
  assert(vm->stackSize < STACK_MAX, "Stack overflow!");
  vm->stack[vm->stackSize++] = value;
}
```

出栈：

```c
Object *pop(VM *vm)
{
  assert(vm->stackSize > 0, "Stack underflow!");
  return vm->stack[--vm->stackSize];
}
```



##### 回收

mark：

```c
void mark(Object *object)
{
  if (object->marked)
    return;

  object->marked = 1;

  if (object->type == OBJ_PAIR)
  {
    mark(object->first);
    mark(object->second);
  }
}
```

由于Pair类型的对象，既可以引用 Int 类型的对象，也可以递归引用 Pair 类型的对象！

因此标记对象的时候，既要递归的标记，又要防止无限制遍历！



markAll：标记栈中所有对象不回收

```c
void markAll(VM *vm)
{
  for (int i = 0; i < vm->stackSize; i++)
  {
    mark(vm->stack[i]);
  }
}
```



gc：垃圾回收主要调用者

```c
void gc(VM *vm)
{
  int numObjects = vm->numObjects;

  markAll(vm);
  sweep(vm);

  vm->maxObjects = vm->numObjects == 0 ? INIT_OBJ_NUM_MAX : vm->numObjects * 2;
  printf("Collected %d objects, %d remaining.\n", numObjects - vm->numObjects, vm->numObjects);
}
```



##### 释放虚拟机

```c
void freeVM(VM *vm)
{
  vm->stackSize = 0;
  gc(vm);
  free(vm);
}
```

此时将栈元素个数设置为0，这样当gc的时候，markAll 不会再将栈内的对象标记为不可回收了；

因此等下一步 sweep 时候，会将链表上所有的对象回收！



##### 测试

```c
void test_pair1()
{
  VM *vm = newVM();				// 创建虚拟机
  pushInt(vm, 1);				// 创建 INT 对象并入栈
  pushInt(vm, 2);				// 创建 INT 对象并入栈
  Object *a = pushPair(vm);		// 利用栈顶两个INT对象创建 Pair 对象并入栈 （此时栈内只有 a 对象，但是堆上有 3 个对象）

  pushInt(vm, 3);
  pushInt(vm, 4);
  Object *b = pushPair(vm);		// 同上

  gc(vm);		// 栈内有两个对象 a, b，堆上有6个对象，但是4个INT对象都被 2 个Pair 对象引用
    			// 因此本次回收 0 个对象

  freeVM(vm);
}
```



##### 参考

http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/

