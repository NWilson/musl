#if defined __wasm32__
#define TP_DTV_AREA_SIZE 8
#elif defined __wasm64__
#define TP_DTV_AREA_SIZE 16
#endif

extern void *__wasm_thread_pointer; // XXX
static inline struct pthread *__pthread_self()
{
	void *tp = __wasm_thread_pointer; // XXX __builtin_thread_pointer();
	return (void*)((char *)(tp) + TP_DTV_AREA_SIZE - sizeof(struct pthread));
}

#define TP_ADJ(p) ((char *)(p) + sizeof(struct pthread) - TP_DTV_AREA_SIZE)
#define TLS_ABOVE_TP

#define MC_PC __ip_dummy
