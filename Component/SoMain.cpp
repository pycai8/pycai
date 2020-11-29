extern void CPycaiLoggerInit();
extern void CPycaiLoggerUninit();
extern void CDllLoaderInit();
extern void CDllLoaderUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CPycaiLoggerInit();
    CDllLoaderInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CPycaiLoggerUninit();
    CDllLoaderUninit();
}

