/*
#define SYSCALL_MACRO(number, func, name, short_desc, long_desc)
*/

// SYSCALL_MACRO( 0, syscall_illegal, "Reserved / null", "", "" )

SYSCALL_MACRO( 0x01,
	syscall_print,
	"print",
	"void print(const char *text)",
	"Tulostaa tekstin. Voimassa toistaiseksi."
)
SYSCALL_MACRO( 0x02,
	syscall_malloc,
	"malloc",
	"void *malloc(size_t size)",
	"Varaa muistia"
)
SYSCALL_MACRO( 0x03,
	syscall_free,
	"free",
	"void free(void *ptr)",
	"Vapauttaa muistia"
)
SYSCALL_MACRO( 0x10,
	syscall_fopen,
	"fopen",
	"FILE * fopen(const char *filename, const char *mode)",
	"Avaa tiedoston"
)
SYSCALL_MACRO( 0x11,
	syscall_fclose,
	"fclose",
	"int fclose(FILE *f)",
	"Sulkee tiedoston"
)
SYSCALL_MACRO( 0x12,
	syscall_fread,
	"fread",
	"int fread(fread_params_t *params)",
	"Lukee tiedostosta puskuriin; typedef struct {void *buf, size_t size, size_t count, FILE *f} fread_params_t;"
)
SYSCALL_MACRO( 0x13,
	syscall_fwrite,
	"fwrite",
	"int fwrite(fwrite_params_t *params)",
	"Kirjoittaa puskurista tiedostoon; typedef struct {const void *buf, size_t size, size_t count, FILE *f} fwrite_params_t;"
)
