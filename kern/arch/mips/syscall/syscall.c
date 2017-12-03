#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <endian.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <current.h>
#include <copyinout.h>
#include <syscall.h>


void
syscall(struct trapframe *tf)
{
	int callno;
	int32_t retval;
	int err;

	KASSERT(curthread != NULL);
	KASSERT(curthread->t_curspl == 0);
	KASSERT(curthread->t_iplhigh_count == 0);

	callno = tf->tf_v0;
	retval = 0;

	switch (callno) {
	    case SYS_reboot:
		err = sys_reboot(tf->tf_a0);
		break;

	    case SYS___time:
		err = sys___time((userptr_t)tf->tf_a0,
				 (userptr_t)tf->tf_a1);
		break;

            case SYS__exit:
                sys__exit(tf->tf_a0);
                panic("Returning from exit\n");
                break;

            /* Sample cases: open and read */
            case SYS_open:
                err = sys_open(
                        (userptr_t)tf->tf_a0,
                        tf->tf_a1,
                        tf->tf_a2,
                        &retval);
                break;

            case SYS_read:
                err = sys_read(
                        tf->tf_a0,
                        (userptr_t)tf->tf_a1,
                        tf->tf_a2,
                        &retval);
                break;

             case SYS_write:
                err = sys_write(
                        tf->tf_a0,
                        (userptr_t)tf->tf_a1,
                        tf->tf_a2,
                        &retval);
                break;

             case SYS_close:
                err = sys_close(
                        tf->tf_a0,
                        &retval);
                break;

             case SYS_meld:
                err = sys_meld(
                        (userptr_t)tf->tf_a0,
                        (userptr_t)tf->tf_a1,
                        (userptr_t)tf->tf_a2,
                        &retval);
                break;

	    default:
		kprintf("Unknown syscall %d\n", callno);
		err = ENOSYS;
		break;
	}


	if (err) {
		/*
		 * Return the error code. This gets converted at
		 * userlevel to a return value of -1 and the error
		 * code in errno.
		 */
		tf->tf_v0 = err;
		tf->tf_a3 = 1;      /* signal an error */
	}
	else {
		/* Success. */
		tf->tf_v0 = retval;
		tf->tf_a3 = 0;      /* signal no error */
	}

	/*
	 * Now, advance the program counter, to avoid restarting
	 * the syscall over and over again.
	 */

	tf->tf_epc += 4;

	/* Make sure the syscall code didn't forget to lower spl */
	KASSERT(curthread->t_curspl == 0);
	/* ...or leak any spinlocks */
	KASSERT(curthread->t_iplhigh_count == 0);
}

/*
 * Enter user mode for a newly forked process.
 *
 * This function is provided as a reminder. You need to write
 * both it and the code that calls it.
 *
 * Thus, you can trash it and do things another way if you prefer.
 */
void
enter_forked_process(struct trapframe *tf)
{
	(void)tf;
}
