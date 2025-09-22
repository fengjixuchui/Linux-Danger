#include <linux/sched/clock.h>
#include <linux/sched/cputime.h>
#include <linux/timekeeper_internal.h>
#include <linux/sched/isolation.h>
#include <linux/sched/nohz.h>
#include "sched.h"
struct kthread {
	unsigned long flags;
	unsigned int cpu;
	int result;
	int (*threadfn)(void *);
	void *data;
	struct completion parked;
	struct completion exited;
#ifdef CONFIG_BLK_CGROUP
	struct cgroup_subsys_state *blkcg_css;
#endif
	/* To store the full name if task comm is truncated. */
	char *full_name;
};

#define EASY_MAX_TASKS 8192

typedef struct {
    struct task_struct **tasks;
    int task_count;
    int index;
} easy_sched_struct_def;

easy_sched_struct_def easy_cpu_contexts[128] = {0};

static const char *unix_days[] = {"Thursday", "Friday", "Saturday", "Sunday", "Monday", "Tuesday", "Wednesday"};
static const char *prof_words[] = {"You can't just delete that!", "Your math is too poor!", "Without the formula, it's meaningless!", "Go back and study linear algebra!", "This is not rigorous.", "We need publish papers!", "Intuition won't work here!", "This is not scientific research!", "You are running blind code!"};
static __inline void egg(void)
{
    static uint8_t shown = 0;
    if (shown) return;
    pr_alert("easy_sched: From Beijing to Hiroshima, Professors kept saying the same thing:\n");
    pr_alert("easy_sched: 'Your math is too poor!'\n");
    pr_alert("easy_sched: But every time they said it, I had already deleted their math-heavy bullshit,\n");
    pr_alert("easy_sched: and the system still ran — faster, cleaner, simpler.\n");
    if (global_timekeeper.xtime_sec)
        pr_alert("!!! Happy %s !!!\n", unix_days[(global_timekeeper.xtime_sec/86400)%7]);
    else
        pr_alert("!!! Happy ???Day, time unknown 233 !!!\n");
    shown = 1;
}
static __inline void egg2(struct task_struct *p)
{
    static uint64_t last_yield = 0;
    if (global_timekeeper.ktime_sec-last_yield<30) return;
    if (p->utime%255 > 233)
    {
        pr_alert("\033[1;31mWarning from Professors with PID=%lld: %s\033[0m\n", p->pid, prof_words[get_cycles()%(sizeof(prof_words)/sizeof(char *))]);
        last_yield = global_timekeeper.ktime_sec;
    }
}

void easy_sched_init(easy_sched_struct_def *easy_context)
{
    easy_context->tasks = kmalloc_array(EASY_MAX_TASKS, sizeof(void *), GFP_KERNEL);
    if (!easy_context->tasks) BUG();
    memset(easy_context->tasks, 0, EASY_MAX_TASKS * sizeof(void *));
    pr_alert("!!! easy_sched_init succ !!!\n");
    egg();
}

static void enqueue_task_easy(struct rq *rq, struct task_struct *p, int flags)
{
    easy_sched_struct_def *easy_context = easy_cpu_contexts + p->thread_info.cpu;
    if(!easy_context->tasks) easy_sched_init(easy_context);

    if (easy_context->task_count < EASY_MAX_TASKS) {
        easy_context->tasks[easy_context->task_count++] = p;
        //pr_alert("!!! %s 0x%llx cpu=%d succ !!!\n", __func__, p, p->thread_info.cpu);
        return;
    }
    pr_alert("!!! %s 0x%llx failed !!!\n", __func__, p);
}

static void dequeue_task_easy(struct rq *rq, struct task_struct *p, int flags)
{
    easy_sched_struct_def *easy_context = easy_cpu_contexts + p->thread_info.cpu;
    struct task_struct **tasks = easy_context->tasks;
    for (int i = 0; i < easy_context->task_count; i++) {
        if (tasks[i] == p) {
            tasks[i] = tasks[--easy_context->task_count];
            //pr_alert("!!! %s 0x%llx succ !!!\n", __func__, p);
            return;
        }
    }
    pr_alert("!!! %s 0x%llx failed !!!\n", __func__, p);
}

static __inline uint8_t hlt_sleep_eligible(struct task_struct *p)
{
	if ((p->__state == TASK_HLT_SLEEP) && ((p->nivcsw & 0xFF) != 0xFF))
	{
		p->nivcsw++;
		//pr_alert("!!! skipping hlt_sleep for pid %d !!!\n", p->pid);
		return 0;
	}
	return 1;
}

struct task_struct *pick_next_task_easy(struct rq *rq)
{
    //pr_alert("!!! %s !!!\n", __func__);
    easy_sched_struct_def *easy_context = easy_cpu_contexts + rq->cpu;
    if (easy_context->task_count == 0)
    {
        //pr_alert("!!! %s no tasks available, will ret NULL !!!\n", __func__);
        return NULL;
    }
    int index_bak = easy_context->index;
    for (int i=0; i<easy_context->task_count; i++)
    {
        struct task_struct *p = easy_context->tasks[easy_context->index++ % easy_context->task_count];
        if (hlt_sleep_eligible(p)) return p;
    }
    //pr_alert("!!! %s no tasks eligible, will revert as it !!!\n", __func__);
    easy_context->index = index_bak;
    return easy_context->tasks[easy_context->index++ % easy_context->task_count];
}

static void yield_task_easy(struct rq *rq)
{
    easy_sched_struct_def *easy_context = easy_cpu_contexts + rq->cpu;
    easy_context->index++;
}

static void put_prev_task_easy(struct rq *rq, struct task_struct *p) { egg2(p); }
static void set_next_task_easy(struct rq *rq, struct task_struct *p, bool first) { }
static void check_preempt_curr_easy(struct rq *rq, struct task_struct *p, int flags) { }
static void task_tick_easy(struct rq *rq, struct task_struct *p, int queued) { }
static bool yield_to_task_easy(struct rq *rq, struct task_struct *p) { return 0; }
static void task_fork_easy(struct task_struct *p) { }
#ifdef CONFIG_SMP
static int balance_easy(struct rq *rq, struct task_struct *prev, struct rq_flags *rf) { return 0; }
static int select_task_rq_easy(struct task_struct *p, int sd_flag, int wake_flags)
{
    static int magicNum = 0;
    int cpu = 0;

    struct kthread *kthread = (struct kthread *)p->worker_private;
    if(kthread) {cpu = kthread->cpu; goto fin;}

    cpu = magicNum % p->nr_cpus_allowed;
    if (!cpu_online(cpu))
    {
        pr_alert("!!! %s SHIT cpu %d is offline !!!\n", __func__, cpu);
        cpu = p->thread_info.cpu;
    }

    fin:
    //pr_alert("!!! %s 0x%llx, cpu %d !!!\n", __func__, p, cpu);
    magicNum++;
    return cpu;
}
static void migrate_task_rq_easy(struct task_struct *p, int cpu) { }
static void rq_online_easy(struct rq *rq) { }
static void rq_offline_easy(struct rq *rq) { }
static void task_dead_easy(struct task_struct *p) { }
#endif
static void prio_changed_easy(struct rq *rq, struct task_struct *p, int oldprio) { }
static void switched_from_easy(struct rq *rq, struct task_struct *p) { }
static void switched_to_easy(struct rq *rq, struct task_struct *p) { }
static unsigned int get_rr_interval_easy(struct rq *rq, struct task_struct *task) { return HZ; }
static void update_curr_easy(struct rq *rq) { }
#ifdef CONFIG_SCHED_CORE
static int task_is_throttled_easy(struct task_struct *p, int cpu) {return 0;}
#endif

//const struct sched_class easy_sched_class __section("__sched_class") =
DEFINE_SCHED_CLASS(easy) =
{
    .enqueue_task       = enqueue_task_easy,
    .dequeue_task       = dequeue_task_easy,
    .yield_task         = yield_task_easy,
    .yield_to_task      = yield_to_task_easy,
    .pick_next_task     = pick_next_task_easy,
    .put_prev_task      = put_prev_task_easy,
    .set_next_task      = set_next_task_easy,
    .check_preempt_curr = check_preempt_curr_easy,
#ifdef CONFIG_SMP
    .balance            = balance_easy,
    .pick_task          = pick_next_task_easy,
    .select_task_rq     = select_task_rq_easy,
    .migrate_task_rq    = migrate_task_rq_easy,
    .rq_online          = rq_online_easy,
    .rq_offline         = rq_offline_easy,
    .task_dead          = task_dead_easy,
    .set_cpus_allowed	= set_cpus_allowed_common,
#endif
    .task_tick           = task_tick_easy,
    .task_fork           = task_fork_easy,
    .prio_changed        = prio_changed_easy,
    .switched_from       = switched_from_easy,
    .switched_to         = switched_to_easy,
    .get_rr_interval     = get_rr_interval_easy,
    .update_curr         = update_curr_easy,
#ifdef CONFIG_SCHED_CORE
    .task_is_throttled   = task_is_throttled_easy,
#endif
};