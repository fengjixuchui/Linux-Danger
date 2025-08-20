#include <linux/sched/clock.h>
#include <linux/sched/cond_resched.h>
#include <linux/sched/cputime.h>
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

// static struct task_struct **tasks = NULL;
// static int task_count = 0;
// static int current_index = 0;

typedef struct {
    struct task_struct **tasks;
    int task_count;
    int index;
} easy_sched_struct_def;

easy_sched_struct_def easy_cpu_contexts[128] = {0};

void easy_sched_init(easy_sched_struct_def *easy_context)
{
    easy_context->tasks = kmalloc_array(EASY_MAX_TASKS, sizeof(struct task_struct *), GFP_KERNEL);
    if (!easy_context->tasks) BUG();
    memset(easy_context->tasks, 0, EASY_MAX_TASKS * sizeof(struct task_struct *));
    pr_alert("!!! easy_sched_init succ !!!\n");
}

static void enqueue_task_easy(struct rq *rq, struct task_struct *p, int flags)
{
    struct kthread *kthread = (struct kthread *)p->worker_private;
    if (kthread) BUG_ON(p->thread_info.cpu != kthread->cpu);

    easy_sched_struct_def *easy_context = easy_cpu_contexts + p->thread_info.cpu;
    if(!easy_context->tasks) easy_sched_init(easy_context);

    if (easy_context->task_count < EASY_MAX_TASKS) {
        easy_context->tasks[easy_context->task_count++] = p;
        p->on_rq = 1;
        rq->nr_running = easy_context->task_count;
        pr_alert("!!! %s 0x%llx cpu=%d succ !!!\n", __func__, p, p->thread_info.cpu);
    }
    else
    {
        pr_alert("!!! %s 0x%llx failed !!!\n", __func__, p);
    }
}

static void dequeue_task_easy(struct rq *rq, struct task_struct *p, int flags)
{
    easy_sched_struct_def *easy_context = easy_cpu_contexts + p->thread_info.cpu;
    struct task_struct **tasks = easy_context->tasks;
    for (int i = 0; i < easy_context->task_count; i++) {
        if (tasks[i] == p) {
            tasks[i] = tasks[--easy_context->task_count];
            p->on_rq = 0;
            rq->nr_running = easy_context->task_count;
            //pr_alert("!!! %s 0x%llx succ !!!\n", __func__, p);
            return;
        }
    }
    pr_alert("!!! %s 0x%llx failed !!!\n", __func__, p);
}

struct task_struct *pick_next_task_easy(struct rq *rq)
{
    //pr_alert("!!! %s !!!\n", __func__);
    easy_sched_struct_def *easy_context = easy_cpu_contexts + rq->cpu;

    if (!cpu_online(rq->cpu))
    {
        pr_alert("!!! %s cpu offline !!!\n", __func__);
        return NULL;
    }
    if (easy_context->task_count == 0)
    {
        //pr_alert("!!! %s no tasks available, will ret NULL !!!\n", __func__);
        return NULL;
    }
    
    struct task_struct *p = easy_context->tasks[easy_context->index % easy_context->task_count];
    //pr_alert("!!! %s 0x%llx !!!\n", __func__, p);
    easy_context->index++;
    return p;
}

static void yield_task_easy(struct rq *rq)
{
    easy_sched_struct_def *easy_context = easy_cpu_contexts + rq->cpu;
    easy_context->index = (easy_context->index + 1) % easy_context->task_count;
}

static void put_prev_task_easy(struct rq *rq, struct task_struct *p) { }
static void set_next_task_easy(struct rq *rq, struct task_struct *p, bool first) { }
static void check_preempt_curr_easy(struct rq *rq, struct task_struct *p, int flags) { }
static void task_tick_easy(struct rq *rq, struct task_struct *p, int queued) { }
static bool yield_to_task_easy(struct rq *rq, struct task_struct *p) { return 0; }
static void task_fork_easy(struct task_struct *p) { }
#ifdef CONFIG_SMP
static int balance_easy(struct rq *rq, struct task_struct *prev, struct rq_flags *rf) { return 0; }
static int select_task_rq_easy(struct task_struct *p, int sd_flag, int wake_flags)
{
    int cpu = task_cpu(p);
    if (!cpu_online(cpu))
        cpu = raw_smp_processor_id();
    pr_alert("!!! %s 0x%llx, cpu %d !!!\n", __func__, p, cpu);
    return cpu;
}
static void migrate_task_rq_easy(struct task_struct *p, int cpu) { }
static void rq_online_easy(struct rq *rq) { }
static void rq_offline_easy(struct rq *rq) { }
static void task_dead_easy(struct task_struct *p) { }
static void set_cpus_allowed_easy(struct task_struct *p, struct affinity_context *ctx) {}
#endif
static void prio_changed_easy(struct rq *rq, struct task_struct *p, int oldprio) { }
static void switched_from_easy(struct rq *rq, struct task_struct *p) { }
static void switched_to_easy(struct rq *rq, struct task_struct *p) { }
static unsigned int get_rr_interval_easy(struct rq *rq, struct task_struct *task) { return HZ; }
static void update_curr_easy(struct rq *rq) { }
#ifdef CONFIG_FAIR_GROUP_SCHED
static void task_change_group_easy(struct task_struct *p) { }
#endif
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

    .check_preempt_curr = check_preempt_curr_easy,

    .pick_next_task     = pick_next_task_easy,
    .put_prev_task      = put_prev_task_easy,
    .set_next_task      = set_next_task_easy,

#ifdef CONFIG_SMP
    .balance            = balance_easy,
    .pick_task          = pick_next_task_easy,
    .select_task_rq     = select_task_rq_easy,
    .migrate_task_rq    = migrate_task_rq_easy,

    .rq_online          = rq_online_easy,
    .rq_offline         = rq_offline_easy,

    .task_dead          = task_dead_easy,
    .set_cpus_allowed	= set_cpus_allowed_easy,
#endif

    .task_tick           = task_tick_easy,
    .task_fork           = task_fork_easy,

    .prio_changed        = prio_changed_easy,
    .switched_from       = switched_from_easy,
    .switched_to         = switched_to_easy,

    .get_rr_interval     = get_rr_interval_easy,
    .update_curr         = update_curr_easy,

#ifdef CONFIG_FAIR_GROUP_SCHED
    .task_change_group   = task_change_group_easy,
#endif

#ifdef CONFIG_SCHED_CORE
    .task_is_throttled   = task_is_throttled_easy,
#endif
};
