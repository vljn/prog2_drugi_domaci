#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct set {
    char naziv[101];
    int godina;
    int tema;
    int broj_delova;
} Set;

typedef struct tema {
    int redni_broj;
    char naziv[101];
    int roditeljska_tema;
} Tema;

typedef struct cvor_set {
    Set *set;
    struct cvor_set *sledeci;
} CvorSet;

typedef struct cvor_tema {
    Tema *tema;
    struct cvor_tema *sledeci;
} CvorTema;

void oslobodi_setove(CvorSet *lista) {
    if (lista) {
        while (lista) {
            CvorSet *t = lista->sledeci;
            free(lista->set);
            free(lista);
            lista = t;
        }
    }
}

void oslobodi_teme(CvorTema *lista) {
    if (lista) {
        while (lista) {
            CvorTema *t = lista->sledeci;
            free(lista->tema);
            free(lista);
            lista = t;
        }
    }
}

Set *ucitaj_set(FILE *f) {
    char linija[128];
    if (!fgets(linija, 128, f)) return NULL;
    Set *set = malloc(sizeof(Set));
    if (!set) {
        printf("MEM_GRESKA");
        return NULL;
    }
    sscanf(linija, "%[^,],%d,%d,%d", set->naziv, &set->godina, &set->tema, &set->broj_delova);
    return set;
}

CvorSet *ucitaj_setove(FILE *f) {
    CvorSet *glava = NULL, *rep = NULL;
    Set *set;
    while ((set = ucitaj_set(f))) {
        CvorSet *novi = malloc(sizeof(CvorSet));
        if (!novi) {
            printf("MEM_GRESKA");
            oslobodi_setove(glava);
            return 0;
        }
        novi->sledeci = NULL;
        novi->set = set;

        if (!glava) {
            glava = novi;
        }
        else {
            rep->sledeci = novi;
        }
        rep = novi;
    }
    return glava;
}

Tema *ucitaj_temu(FILE *f) {
    char linija[128];
    if (!fgets(linija, 128, f)) return NULL;
    Tema *tema = malloc(sizeof(Tema));
    if (!tema) {
        printf("MEM_GRESKA");
        return NULL;
    }
    sscanf(linija, "%d,%[^,],%d", &tema->redni_broj, tema->naziv, &tema->roditeljska_tema);
    return tema;
}

int dodaj_temu(CvorTema **lista, Tema *t) {
    CvorTema *novi = malloc(sizeof(CvorTema));
    if (!novi) {
        printf("MEM_GRESKA");
        return 0;
    }
    novi->sledeci = NULL;
    novi->tema = t;
    CvorTema *tr = *lista, *pr = NULL;

    while (tr) {
        if (tr->tema->roditeljska_tema > t->roditeljska_tema ||
           (tr->tema->roditeljska_tema == t->roditeljska_tema &&
            tr->tema->redni_broj > t->redni_broj)) {
            if (pr) pr->sledeci = novi;
            else *lista = novi;
            novi->sledeci = tr;
            return 1;
            }
        pr = tr;
        tr = tr->sledeci;
    }

    if (pr) pr->sledeci = novi;
    else *lista = novi;

    return 1;
}

CvorTema *ucitaj_teme(FILE *f) {
    CvorTema *glava = NULL;
    Tema *t;
    while ((t = ucitaj_temu(f))) {
        if (!dodaj_temu(&glava, t)) {
            free(t);
            oslobodi_teme(glava);
            return NULL;
        }
    }

    return glava;
}

void ispisi_setove(FILE *f, CvorSet *setovi, int n) {
    while (setovi && n) {
        Set *s = setovi->set;
        fprintf(f, "%s %d %d %d\n", s->naziv, s->godina, s->tema, s->broj_delova);

        n--;
        setovi = setovi->sledeci;
    }
}

void ispisi_teme(FILE *f, CvorTema *teme) {
    fputc('\n', f);
    while (teme) {
        fprintf(f, "%d %s %d\n", teme->tema->redni_broj, teme->tema->naziv, teme->tema->roditeljska_tema);

        teme = teme->sledeci;
    }
}

Tema *nadji_temu(CvorTema *teme, int redni_broj) {
    while (teme) {
        if (teme->tema->redni_broj == redni_broj) {
            return teme->tema;
        }

        teme = teme->sledeci;
    }
}

void ispisi_filtrirane_setove(FILE *f, CvorSet *setovi, CvorTema *teme) {
    fputc('\n', f);
    while (setovi) {
        Set *s = setovi->set;
        if (s->godina < 2000 || s->broj_delova < 1000) {
            setovi = setovi->sledeci;
            continue;
        }

        Tema *tema = nadji_temu(teme, s->tema);
        Tema *roditeljska = nadji_temu(teme, tema->roditeljska_tema);
        char objedinjeno[203] = {0};
        if (roditeljska) {
            strcat(objedinjeno, roditeljska->naziv);
            strcat(objedinjeno, ": ");
        }
        strcat(objedinjeno, tema->naziv);

        fprintf(f, "%-30s %-25s %4d %4d\n", s->naziv, objedinjeno, s->godina, s->broj_delova);
        setovi = setovi->sledeci;
    }
}

int main(void) {
    FILE *f_ulaz_setovi = fopen("sets.csv", "r");
    if (!f_ulaz_setovi) {
        printf("DAT_GRESKA");
        return 0;
    }
    FILE *f_ulaz_teme = fopen("themes.csv", "r");
    if (!f_ulaz_teme) {
        printf("DAT_GRESKA");
        return 0;
    }
    FILE *f_izlaz = fopen("lego.txt", "w");
    if (!f_izlaz) {
        printf("DAT_GRESKA");
        return 0;
    }
    int n;
    scanf("%d", &n);

    CvorSet *setovi = ucitaj_setove(f_ulaz_setovi);
    if (!setovi) {
        fclose(f_ulaz_setovi);
        fclose(f_ulaz_teme);
        fclose(f_izlaz);
        return 0;
    }
    ispisi_setove(f_izlaz, setovi, n);

    CvorTema *teme = ucitaj_teme(f_ulaz_teme);
    if (!teme) {
        fclose(f_ulaz_setovi);
        fclose(f_ulaz_teme);
        fclose(f_izlaz);
        oslobodi_setove(setovi);
        return 0;
    }
    ispisi_teme(f_izlaz, teme);

    ispisi_filtrirane_setove(f_izlaz, setovi, teme);

    fclose(f_ulaz_setovi);
    fclose(f_ulaz_teme);
    fclose(f_izlaz);
    oslobodi_setove(setovi);
    oslobodi_teme(teme);

    return 0;
}