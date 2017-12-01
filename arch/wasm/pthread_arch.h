static inline struct pthread *__pthread_self(void) { return pthread_self(); }

#define TP_ADJ(p) (p)

#define MC_PC __ip_dummy
