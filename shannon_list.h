#ifndef __SHANNON_LIST_H
#define	__SHANNON_LIST_H

struct shannon_list_head {
	struct shannon_list_head *next, *prev;
};

#define SHANNON_LIST_HEAD_INIT(name) { &(name), &(name) }

static inline void SHANNON_INIT_LIST_HEAD(struct shannon_list_head *list)
{
	list->next = list;
	list->prev = list;
}


static inline void __shannon_list_add(struct shannon_list_head *new,
			      struct shannon_list_head *prev,
			      struct shannon_list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}


static inline void shannon_list_add(struct shannon_list_head *new, struct shannon_list_head *head)
{
	__shannon_list_add(new, head, head->next);
}


static inline void shannon_list_add_tail(struct shannon_list_head *new, struct shannon_list_head *head)
{
	__shannon_list_add(new, head->prev, head);
}

static inline void __shannon_list_del(struct shannon_list_head * prev, struct shannon_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void __shannon_list_del_entry(struct shannon_list_head *entry)
{
	__shannon_list_del(entry->prev, entry->next);
}

static inline void shannon_list_del(struct shannon_list_head *entry)
{
	__shannon_list_del(entry->prev, entry->next);
	SHANNON_INIT_LIST_HEAD(entry);
}

static inline void shannon_list_del_init(struct shannon_list_head *entry)
{
	__shannon_list_del_entry(entry);
	SHANNON_INIT_LIST_HEAD(entry);
}

static inline int shannon_list_empty(const struct shannon_list_head *head)
{
	return head->next == head;
}

static inline int shannon_list_empty_careful(const struct shannon_list_head *head)
{
	struct shannon_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}


#define shannon_list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define shannon_list_first_entry(ptr, type, member) \
	shannon_list_entry((ptr)->next, type, member)

#define shannon_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define shannon_list_for_each_entry(pos, head, member)				\
	for (pos = shannon_list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = shannon_list_entry(pos->member.next, typeof(*pos), member))

#define shannon_list_for_each_entry_reverse(pos, head, member)			\
	for (pos = shannon_list_entry((head)->prev, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = shannon_list_entry(pos->member.prev, typeof(*pos), member))

#define shannon_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = shannon_list_entry((head)->next, typeof(*pos), member),	\
		n = shannon_list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = shannon_list_entry(n->member.next, typeof(*n), member))

#endif /* __SHANNON_LIST_H */
