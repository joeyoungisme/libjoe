

```text

    JoeYog                                 JoeYogPkg                                JoeYogPkg
   _____________________                  _____________________                     _____________________
  | [ method ]          |                | [ method ]          | <-----------o     | [ method ]          | <-----------o
  |  send_package (...) |                |  set_data (...)     |             |     |  set_data (...)     |             |
  |  recv_package (...) |                |  get_data (...)     |             |     |  get_data (...)     |             |
  |  get_error (...)    |                |  get_data_len (...) |             |     |  get_data_len (...) |             |
  |  ...                |                |  ...                |             |     |  ...                |             |
  |_____________________|                |_____________________|             |     |_____________________|             |
  | [ priv member ]     |                | [ priv member ]     |             |     | [ priv member ]     |             |
  | List_t recv_...     |                | uint8_t data[...]   |             |     | uint8_t data[...]   |             |
  | List_t send_...     |                | ListItem_t item     |             |     | ListItem_t item     |             |
  |v____________________|                |_v___________________|             |     |_v___________________|             |
   v                                       v                                 |       v                                 |
   v List_t                                v                                 |       v                                 |
   _____________________________           v                                 |       v                                 |
  | [ member ]                  | <--o     v                                 |       v                                 |
  | UBaseType_t uxNumberOfItems |    |     v                                 |       v                                 |
  | ListItem_t *pxIndex         |    |     v                                 |       v                                 |
  | MiniListItem_t xListEnd     |    |     v                                 |       v                                 |
  |v____________________________|    |     v                                 |       v                                 |
   v                                 |     v                                 |       v                                 |
   v MiniListItem_t                  |     v ListItem_t                      |       v ListItem_t                      |
   _______________________________   |     _______________________________   |       _______________________________   |
  | [ member ]                    |  | o->| [ member ]                    |<-|-o o->| [ member ]                    |  |
  | TickType_t xItemValue         |  | |  | TickType_t xItemValue         |  | | |  | TickType_t xItemValue         |  |
  | struct xLIST_ITEM *pxNext ----|--|-o  | struct xLIST_ITEM *pxNext     |--|-|-o  | struct xLIST_ITEM *pxNext     |  |
  | struct xLIST_ITEM *pxPrevious |  |    | struct xLIST_ITEM *pxPrevious |  | o----| struct xLIST_ITEM *pxPrevious |  |
  |_______________________________|  |    |                               |  |      |_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|  |
                                     |    | void *pvOwner                 |--o      | void *pvOwner ----------------|--o
                                     o----| struct xLIST *pxContainer     |---------| struct xLIST *pxContainer     |
                                          |_______________________________|         |_______________________________|
  * MiniListItem_t and ListItem_t
    first helf part is same, so I think ...
    maybe we can call MiniListItem_t to ListItem_t header

  * "xItemValue" is simulate "priority"
    vListInsert(List_t *, ListItem_t *)
    this function will sorting item by xItemValue (priority)

```
