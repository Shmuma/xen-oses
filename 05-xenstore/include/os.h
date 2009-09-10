#ifndef __OS_H__
#define __OS_H__


#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif
#define unlikely(x)  __builtin_expect((x),0)
#define likely(x)  __builtin_expect((x),1)


#endif	/* __OS_H__ */
