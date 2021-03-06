/* ZEDA - Elementary Data and Algorithms
 * Copyright (C) 1998 Tomomichi Sugihara (Zhidao)
 */
/*! \file zeda_heap.h
 * \brief heap operation.
 * \author Zhidao
 */

#ifndef __ZEDA_HEAP_H__
#define __ZEDA_HEAP_H__

#include <zeda/zeda_misc.h>

__BEGIN_DECLS

/* ********************************************************** */
/*! \defgroup dheap dynamically-allocated heap
 * \{ *//* ************************************************** */

/* ********************************************************** */
/*! \def zHeapClass(node_t,data_t)
 * \brief generate binary tree heap class.
 *
 * A macro zHeapClass() generates a new binary tree heap class
 * and prototypes of some associated methods.
 *
 * The heap class \a heap_t defines a node that contains
 * data with the type \a data_t and two pointers to children.
 *
 * The methods to be generated are (node_t)IsEmpty(), (node_t)Init(),
 * (node_t)Destroy(), (node_t)Add() and (node_t)Delete(), where
 * (node_t)s are replaced by the actual type name. The body
 * implementation of those functions are generated by calling
 * zHeapClassMethod(node_t, data_t);
 *//* ******************************************************* */

#define zHeapClass(node_t,data_t) \
typedef struct __##node_t{\
  struct __##node_t *child[2];\
  uint size;\
  data_t data;\
} node_t;\
\
__EXPORT bool node_t##IsEmpty(node_t *tree);\
__EXPORT node_t *node_t##Init(node_t *node);\
__EXPORT void node_t##Destroy(node_t *tree);\
__EXPORT node_t *node_t##Add(node_t *tree, data_t val, int (* cmp)(node_t*,node_t*,void*), void *util);\
__EXPORT node_t *node_t##Delete(node_t *tree, int (* cmp)(node_t*,node_t*,void*), void *util)

#define zHeapClassMethod(node_t,data_t) \
bool node_t##IsEmpty(node_t *tree){\
  return tree->size == 0;\
}\
\
node_t *node_t##Init(node_t *node){\
  node->child[0] = node->child[1] = NULL;\
  node->size = 0;\
  return node;\
}\
\
static uint __##node_t##MaskInit(node_t *tree){\
  uint mask;\
  mask = 1 << ( sizeof(int)*8 - 2 );\
  while( !( mask & tree->size ) ) mask >>= 1;\
  return mask >> 1;\
}\
\
static void __##node_t##NodeDestroy(node_t *node){\
  if( node->child[0] )\
    __##node_t##NodeDestroy( node->child[0] );\
  if( node->child[1] )\
    __##node_t##NodeDestroy( node->child[1] );\
  free( node );\
}\
\
void node_t##Destroy(node_t *tree){\
  if( tree->child[0] )\
    __##node_t##NodeDestroy( tree->child[0] );\
  node_t##Init( tree );\
}\
\
static node_t *__##node_t##NodeAlloc(data_t val){\
  node_t *node;\
  if( !( node = zAlloc( node_t, 1 ) ) ){\
    ZALLOCERROR();\
    return NULL;\
  }\
  node_t##Init( node );\
  memcpy( &node->data, &val, sizeof(data_t) );\
  return node;\
}\
\
static void __##node_t##NodeSwap(node_t *parent, int id, node_t *node, int cid){\
  node_t *tmp;\
  parent->child[id] = node->child[cid];\
  node->child[cid] = node->child[cid]->child[cid];\
  parent->child[id]->child[cid] = node;\
  tmp = node->child[1-cid];\
  node->child[1-cid] = parent->child[id]->child[1-cid];\
  parent->child[id]->child[1-cid] = tmp;\
}\
\
static node_t *__##node_t##NodeAdd(node_t *parent, int id, node_t *node, node_t *node_new, uint mask, uint size, int (* cmp)(node_t*,node_t*,void*), void *util){\
  uint cid;\
  cid = mask & size ? 1 : 0;\
  if( mask == 1 )\
    node->child[cid] = node_new;\
  else\
    __##node_t##NodeAdd( node, cid, node->child[cid], node_new, mask >> 1, size, cmp, util );\
  if( !cmp( node, node->child[cid], util ) )\
    __##node_t##NodeSwap( parent, id, node, cid );\
  return node_new;\
}\
\
node_t *node_t##Add(node_t *tree, data_t val, int (* cmp)(node_t*,node_t*,void*), void *util){\
  node_t *np_new;\
  uint mask;\
  if( !( np_new = __##node_t##NodeAlloc( val ) ) ) return NULL;\
  if( ++tree->size == 1 ){\
    tree->child[0] = np_new;\
    return tree->child[0];\
  }\
  mask = __##node_t##MaskInit( tree );\
  return __##node_t##NodeAdd( tree, 0, tree->child[0], np_new, mask, tree->size, cmp, util );\
}\
\
static void __##node_t##Down(node_t *parent, int id, node_t *node, int (* cmp)(node_t*,node_t*,void*), void *util){\
  uint cid;\
  if( !node->child[0] ) return;\
  cid = !node->child[1] ? 0 : ( cmp( node->child[0], node->child[1], util ) ? 0 : 1 );\
  if( !cmp( node, node->child[cid], util ) ){\
    __##node_t##NodeSwap( parent, id, node, cid );\
    __##node_t##Down( parent->child[id], cid, node, cmp, util );\
  }\
}\
\
node_t *node_t##Delete(node_t *tree, int (* cmp)(node_t*,node_t*,void*), void *util){\
  node_t *ret, *np;\
  uint mask, cid;\
  mask = __##node_t##MaskInit( tree );\
  for( np=tree->child[0]; np; mask>>=1, np=np->child[cid] ){\
    cid = mask & tree->size ? 1 : 0;\
    if( mask == 1 ) break;\
  }\
  ret = tree->child[0];\
  if( !np ){\
    tree->child[0] = NULL;\
  } else if( ( tree->child[0] = np->child[cid] ) ){\
    np->child[cid] = NULL;\
    tree->child[0]->child[0] = ret->child[0];\
    tree->child[0]->child[1] = ret->child[1];\
    __##node_t##Down( tree, 0, tree->child[0], cmp, util );\
  }\
  ret->child[0] = ret->child[1] = NULL;\
  tree->size--;\
  return ret;\
}

/*! \} */

__END_DECLS

#endif /* __ZEDA_HEAP_H__ */
