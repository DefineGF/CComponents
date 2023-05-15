#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX 256      /* 栈容量 */
#define INIT_OBJ_NUM_MAX 8 /* 触发GC 回收 */

typedef enum
{
  OBJ_INT,
  OBJ_PAIR
} ObjectType;

typedef struct sObject
{
  ObjectType type;
  unsigned char marked;
  struct sObject *next;
  union
  {
    int value;
    struct
    {
      struct sObject *first;
      struct sObject *second;
    };
  };
} Object;

typedef struct
{
  Object *stack[STACK_MAX];
  int stackSize;

  Object *firstObject;

  int numObjects;
  int maxObjects;
} VM;

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

void assert(int condition, const char *message)
{
  if (!condition)
  {
    printf("error: %s\n", message);
    exit(1);
  }
}

VM *newVM()
{
  VM *vm = malloc(sizeof(VM));
  vm->stackSize = 0;
  vm->firstObject = NULL;
  vm->numObjects = 0;
  vm->maxObjects = INIT_OBJ_NUM_MAX;
  return vm;
}

void push(VM *vm, Object *value)
{
  assert(vm->stackSize < STACK_MAX, "Stack overflow!");
  vm->stack[vm->stackSize++] = value;
}

Object *pop(VM *vm)
{
  assert(vm->stackSize > 0, "Stack underflow!");
  return vm->stack[--vm->stackSize];
}

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

void markAll(VM *vm)
{
  for (int i = 0; i < vm->stackSize; i++)
  {
    mark(vm->stack[i]);
  }
}

void sweep(VM *vm)
{
  Object **object = &vm->firstObject;
  while (*object)
  {
    if (!(*object)->marked)
    {
      Object *unreached = *object;
      *object = unreached->next;
      free(unreached);
      vm->numObjects--;
    }
    else
    {
      (*object)->marked = 0;
      object = &(*object)->next;
    }
  }
}

void gc(VM *vm)
{
  int numObjects = vm->numObjects;

  markAll(vm);
  sweep(vm);

  vm->maxObjects = vm->numObjects == 0 ? INIT_OBJ_NUM_MAX : vm->numObjects * 2;
  printf("Collected %d objects, %d remaining.\n", numObjects - vm->numObjects, vm->numObjects);
}

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

void log_vm(VM *vm)
{
  printf("stack_size: %d\tcount: %d\tmax_count: %d\n",
         vm->stackSize, vm->numObjects, vm->maxObjects);
}

void pushInt(VM *vm, int intValue)
{
  Object *object = newObject(vm, OBJ_INT);
  object->value = intValue;

  push(vm, object);
}

Object *pushPair(VM *vm)
{
  Object *object = newObject(vm, OBJ_PAIR);
  object->second = pop(vm);
  object->first = pop(vm);

  push(vm, object);
  return object;
}

void objectPrint(Object *object)
{
  switch (object->type)
  {
  case OBJ_INT:
    printf("%d\n", object->value);
    break;

  case OBJ_PAIR:
    printf("(");
    objectPrint(object->first);
    printf(", ");
    objectPrint(object->second);
    printf(")\n");
    break;
  }
}

void freeVM(VM *vm)
{
  vm->stackSize = 0;
  gc(vm);
  free(vm);
}

void print_ls(VM *vm)
{
  printf("log list: \n");
  Object **pObj = &vm->firstObject;
  while ((*pObj))
  {
    printf("mark: %d,\t value: %d\n", (*pObj)->marked, (*pObj)->value);
    pObj = &(*pObj)->next;
  }
}

void test_int()
{
  int temp = -1;

  VM *vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);

  temp = pop(vm)->value;

  pushInt(vm, 3);
  pushInt(vm, 4);

  print_ls(vm);

  gc(vm);

  print_ls(vm);

  freeVM(vm);
}

void test_pair1()
{
  VM *vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  Object *a = pushPair(vm);

  pushInt(vm, 3);
  pushInt(vm, 4);
  Object *b = pushPair(vm);

  gc(vm);
  log_vm(vm);

  freeVM(vm);
}

void test_pair2()
{
  VM *vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  Object *a = pushPair(vm);

  pushInt(vm, 3);
  pushInt(vm, 4);
  Object *b = pushPair(vm);

  a->first = b;
  b->first = a;

  gc(vm);
  log_vm(vm);

  freeVM(vm);
}

int main(int argc, const char *argv[])
{
  // test_int();
  test_pair1();
  printf("\n");
  test_pair2();
  return 0;
}
