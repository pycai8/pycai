extern void CPycaiLoggerInit();
extern void CPycaiLoggerUninit();
extern void CSoLoaderInit();
extern void CSoLoaderUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CPycaiLoggerInit();
    CSoLoaderInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CPycaiLoggerUninit();
    CSoLoaderUninit();
}

