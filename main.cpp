#include <iostream>
#include <graphics.h>
#include <winbgim.h>
#include <windows.h>
#include <mmsystem.h>
#include <cstring>
#include <ctime>

using namespace std;

enum TipJoc
{
    PLAYER_VS_PLAYER,
    PLAYER_VS_EASY,
    PLAYER_VS_HARD
};
TipJoc tipJoc;

enum Dimensiune
{
    SMALL,
    MEDIUM,
    BIG
};

enum Culoare
{
    NICIUNA,
    PORTOCALIU,
    ALBASTRU
};

struct Piesa
{
    Dimensiune dimensiune;
    Culoare culoare;
};

struct Pozitie
{
    Piesa piese[5];
    int totalPiese;
};
Pozitie tabla[3][3];

struct Jucator
{
    Culoare culoare;
    int pieseMici = 2;
    int pieseMedii = 2;
    int pieseMari = 2;
};
Jucator jucatorCurent, jucatorUrmator, jucatorulAlbastru, jucatorulPortocaliu;

struct punct
{
    int x, y;
} C;

struct dreptunghi
{
    punct SS, DJ;
};

bool apartine(punct P, dreptunghi D)
{
    return D.SS.x <= P.x && P.x <= D.DJ.x+75 && D.SS.y <= P.y && P.y <= D.DJ.y;
}

struct buton
{
    dreptunghi D;
    int culoare;
    char text[20];
};
buton B[5];

int nrButoane = 4, butonulApasat, comanda = 0, linieSelectata, coloanaSelectata, marimePiesa = 0;
char sunet[10] = "click.wav", nimic[10];
bool apasatInapoiLaMeniu, apasatResetare, piesaDePeTabla=false, v[6], randPlayer = true;

void deseneazaMeniul()
{
    int i;
    readimagefile("banner.jpg", 200, 0, 800, 300 );
    outtextxy(10, 770, "Balcan Bruno-Cristian & Chilariu Bianca-Gabriela & Dragos Gabriel-Catalin");

    for(i = 1; i <= nrButoane; i++)
    {
        B[i].D.SS.x = 475;
        B[i].D.DJ.x = 475;
        B[i].D.SS.y = 135*i+100;
        B[i].D.DJ.y = 135*i+150;
        switch(i)
        {
        case 1:
            strcpy(B[i].text, "Joc");
            break;
        case 2:
            strcpy(B[i].text, "Setari");
            break;
        case 3:
            strcpy(B[i].text, "Reguli");
            break;
        case 4:
            strcpy(B[i].text, "Iesire");
            break;
        }
        rectangle(B[i].D.SS.x, B[i].D.SS.y, B[i].D.DJ.x+75, B[i].D.DJ.y);
        outtextxy(B[i].D.SS.x+15, B[i].D.SS.y+10, B[i].text);
    }
}

int butonAles()
{
    int i;
    punct p;

    if(ismouseclick(WM_LBUTTONDOWN))
    {
        clearmouseclick(WM_LBUTTONDOWN);
        p.x = mousex();
        p.y = mousey();

        for(i = 1; i <= nrButoane; i++)
            if(apartine(p, B[i].D))
            {
                PlaySound(TEXT(sunet), NULL, SND_ASYNC);
                return i;
            }
    }
    return 0;
}

void initializeazaTabla()
{
    for(int i = 0; i <= 2; i++)
        for(int j = 0; j <= 2; j++)
            tabla[i][j].totalPiese = 0;
}

void initializeazaNrPieseFrontEnd()
{
    for(int i = 0; i <= 5; i++)
        v[i] = false;
}

void reset()
{
    initializeazaNrPieseFrontEnd();
    initializeazaTabla();
    jucatorCurent.culoare = PORTOCALIU;
    jucatorUrmator.culoare = ALBASTRU;
    jucatorulAlbastru.pieseMari = jucatorulAlbastru.pieseMedii = jucatorulAlbastru.pieseMici = 2;
    jucatorulPortocaliu.pieseMari = jucatorulPortocaliu.pieseMedii = jucatorulPortocaliu.pieseMici = 2;
}

void inapoiLaMeniu()
{
    punct p;

    if(ismouseclick(WM_LBUTTONDOWN))
    {
        p.x = mousex();
        p.y = mousey();

        if(470 <= p.x && 695 <= p.y && p.x <= 578 && p.y <= 720)
        {
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            apasatInapoiLaMeniu = false;
            setfillstyle(SOLID_FILL, BLACK);
            bar(0, 0, 1100, 790);
            deseneazaMeniul();
            comanda = 0;
            reset();
        }
    }
}

void adaugaPiesaPePozitie(Pozitie& pozitie, Piesa piesa)
{
    pozitie.totalPiese++;
    pozitie.piese[pozitie.totalPiese] = piesa;
}

void scoatePiesaDePePozitie(Pozitie& pozitie)
{
    pozitie.totalPiese--;
}

Piesa piesaDinVarf(Pozitie pozitie)
{
    return pozitie.piese[pozitie.totalPiese];
}

void mutaPiesaDePeTablaBackEnd(int linieSursa, int coloanaSursa, int linieDestinatie, int coloanaDestinatie, Jucator jucator)
{
    if(tabla[linieSursa][coloanaSursa].totalPiese > 0)
    {
        Piesa piesaDinVarfSursa = piesaDinVarf(tabla[linieSursa][coloanaSursa]);
        if(piesaDinVarfSursa.culoare == jucator.culoare)
        {

            Piesa piesaDinVarfDestinatie = piesaDinVarf(tabla[linieDestinatie][coloanaDestinatie]);
            if(tabla[linieDestinatie][coloanaDestinatie].totalPiese == 0 || piesaDinVarfDestinatie.dimensiune < piesaDinVarfSursa.dimensiune)
            {
                adaugaPiesaPePozitie(tabla[linieDestinatie][coloanaDestinatie], piesaDinVarfSursa);
                scoatePiesaDePePozitie(tabla[linieSursa][coloanaSursa]);
            }
        }
    }
}

Culoare cine_e_castigatorul()
{
    Culoare culoareLinie = NICIUNA;
    for(int linie = 0; linie <= 2; linie++)
    {

        if(tabla[linie][0].totalPiese > 0 && tabla[linie][1].totalPiese > 0 && tabla[linie][2].totalPiese > 0)
        {
            Piesa piesaColoana0 = piesaDinVarf(tabla[linie][0]);
            Piesa piesaColoana1 = piesaDinVarf(tabla[linie][1]);
            Piesa piesaColoana2 = piesaDinVarf(tabla[linie][2]);

            if(piesaColoana0.culoare == piesaColoana1.culoare && piesaColoana1.culoare == piesaColoana2.culoare)
                if(piesaColoana0.culoare != jucatorUrmator.culoare)
                    return piesaColoana0.culoare;
                else
                    culoareLinie = piesaColoana0.culoare;
        }
    }

    for(int coloana = 0; coloana <= 2; coloana++)
    {

        if(tabla[0][coloana].totalPiese > 0 && tabla[1][coloana].totalPiese > 0 && tabla[2][coloana].totalPiese > 0)
        {

            Piesa piesaLinie0 = piesaDinVarf(tabla[0][coloana]);
            Piesa piesaLinie1 = piesaDinVarf(tabla[1][coloana]);
            Piesa piesaLinie2 = piesaDinVarf(tabla[2][coloana]);

            if(piesaLinie0.culoare == piesaLinie1.culoare && piesaLinie1.culoare == piesaLinie2.culoare)
                if(piesaLinie0.culoare != jucatorUrmator.culoare)
                    return piesaLinie0.culoare;
                else
                    culoareLinie = piesaLinie0.culoare;
        }
    }

    if(tabla[0][0].totalPiese > 0 && tabla[1][1].totalPiese > 0 && tabla[2][2].totalPiese > 0)
    {
        Piesa piesa0 = piesaDinVarf(tabla[0][0]);
        Piesa piesa1 = piesaDinVarf(tabla[1][1]);
        Piesa piesa2 = piesaDinVarf(tabla[2][2]);

        if(piesa0.culoare == piesa1.culoare && piesa1.culoare == piesa2.culoare)
            if(piesa0.culoare != jucatorUrmator.culoare)
                return piesa0.culoare;
            else
                culoareLinie = piesa0.culoare;
    }

    if(tabla[0][2].totalPiese > 0 && tabla[1][1].totalPiese > 0 && tabla[2][0].totalPiese > 0)
    {
        Piesa piesa0 = piesaDinVarf(tabla[0][2]);
        Piesa piesa1 = piesaDinVarf(tabla[1][1]);
        Piesa piesa2 = piesaDinVarf(tabla[2][0]);

        if(piesa0.culoare == piesa1.culoare && piesa1.culoare == piesa2.culoare)
            if(piesa0.culoare != jucatorUrmator.culoare)
                return piesa0.culoare;
            else
                culoareLinie = piesa0.culoare;
    }
    return culoareLinie;
}

void initializeazaCuloareJucatori()
{
    jucatorCurent.culoare = PORTOCALIU;
    jucatorUrmator.culoare = ALBASTRU;
    jucatorulAlbastru.culoare = ALBASTRU;
    jucatorulPortocaliu.culoare = PORTOCALIU;
}

void selecteazaPiesaUnuiJucatorBackEnd(Jucator &jucator, Dimensiune dimensiunePiesaAleasa)
{
    if(dimensiunePiesaAleasa == BIG)
        jucator.pieseMari--;

    else if(dimensiunePiesaAleasa == MEDIUM)
        jucator.pieseMedii--;

    else
        jucator.pieseMici--;
}

bool pieseDisponibile(Jucator jucator, Dimensiune dimensiunePiesaAleasa)
{

    if(dimensiunePiesaAleasa == BIG)
    {
        if(jucator.pieseMari > 0)
            return true;
        else
            return false;
    }
    else if(dimensiunePiesaAleasa == MEDIUM)
    {
        if(jucator.pieseMedii > 0)
            return true;
        else
            return false;
    }
    else if(jucator.pieseMici > 0)
        return true;

    return false;
}

void actualizeazaPieseDisponibileFrontEnd(Piesa piesa)
{
    if(piesa.culoare == PORTOCALIU)
    {
        if(piesa.dimensiune == SMALL)
        {
            if(v[0] == false)
            {
                v[0] = true;
                outtextxy(90, 220, "x1");
            }
            else
                outtextxy(90, 220, "x0");
        }
        else if(piesa.dimensiune == MEDIUM)
        {
            if(v[1] == false)
            {
                v[1] = true;
                outtextxy(90, 367, "x1");
            }
            else
                outtextxy(90, 367, "x0");
        }
        else
        {
            if(v[2] == false)
            {
                v[2] = true;
                outtextxy(90, 528, "x1");
            }
            else
                outtextxy(90, 528, "x0");
        }
    }
    else
    {
        if(piesa.dimensiune == SMALL)
        {
            if(v[3] == false)
            {
                v[3] = true;
                outtextxy(990, 220, "x1");
            }
            else
                outtextxy(990, 220, "x0");
        }
        else if(piesa.dimensiune == MEDIUM)
        {
            if(v[4] == false)
            {
                v[4] = true;
                outtextxy(990, 367, "x1");
            }
            else
                outtextxy(990, 367, "x0");
        }
        else
        {
            if(v[5] == false)
            {
                v[5] = true;
                outtextxy(990, 528, "x1");
            }
            else
                outtextxy(990, 528, "x0");
        }
    }
}

void mutaPiesaDePeTablaFrontEnd(int x, int y, char* numeFisier, Dimensiune dimensiune)
{
    if(dimensiune == BIG)
        readimagefile(numeFisier, x/150*150+10, y/150*150+10, x/150*150+140, y/150*150+140);

    else if(dimensiune == MEDIUM)
        readimagefile(numeFisier, x/150*150+41, y/150*150+41, x/150*150+109, y/150*150+109);

    else
        readimagefile(numeFisier, x/150*150+57, y/150*150+57, x/150*150+93, y/150*150+93);

    setfillstyle(SOLID_FILL, BLACK);
    bar((coloanaSelectata+2)*150+1, (linieSelectata+1)*150+1, (coloanaSelectata+2)*150+150, (linieSelectata+1)*150+150);

    if(tabla[linieSelectata][coloanaSelectata].totalPiese != 0)
    {
        if(piesaDinVarf(tabla[linieSelectata][coloanaSelectata]).dimensiune == BIG)
        {
            if(piesaDinVarf(tabla[linieSelectata][coloanaSelectata]).culoare == ALBASTRU)
                readimagefile("blue_fara_numar.jpg", (coloanaSelectata+2)*150+10, (linieSelectata+1)*150+10, (coloanaSelectata+2)*150+140, (linieSelectata+1)*150+140);

            else
                readimagefile("orange_fara_numar.jpg", (coloanaSelectata+2)*150+10, (linieSelectata+1)*150+10, (coloanaSelectata+2)*150+140, (linieSelectata+1)*150+140);
        }
        else if(piesaDinVarf(tabla[linieSelectata][coloanaSelectata]).dimensiune == MEDIUM)
        {
            if(piesaDinVarf(tabla[linieSelectata][coloanaSelectata]).culoare == ALBASTRU)
                readimagefile("blue_fara_numar.jpg", (coloanaSelectata+2)*150+41, (linieSelectata+1)*150+41, (coloanaSelectata+2)*150+109, (linieSelectata+1)*150+109);

            else
                readimagefile("orange_fara_numar.jpg", (coloanaSelectata+2)*150+41, (linieSelectata+1)*150+41, (coloanaSelectata+2)*150+109, (linieSelectata+1)*150+109);

        }
        else
        {
            if(piesaDinVarf(tabla[linieSelectata][coloanaSelectata]).culoare == ALBASTRU)
                readimagefile("blue_fara_numar.jpg", (coloanaSelectata+2)*150+57, (linieSelectata+1)*150+57, (coloanaSelectata+2)*150+93, (linieSelectata+1)*150+93);

            else
                readimagefile("orange_fara_numar.jpg", (coloanaSelectata+2)*150+57, (linieSelectata+1)*150+57, (coloanaSelectata+2)*150+93, (linieSelectata+1)*150+93);
        }
    }
}
void selectarePiesaJucatorFrontEnd(Jucator& jucator)
{
    int X1,X2,X3,Y1,Y2,Y3;

    if(jucator.culoare == PORTOCALIU)
    {
        X1 = X2 = X3 = 110;
        Y1 = 140;
        Y2 = 170;
        Y3 = 200;
    }
    else
    {
        Y1 = Y2 = Y3 = 990;
        X1 = 950;
        X2 = 930;
        X3 = 900;
    }

    if(ismouseclick(WM_LBUTTONDOWN))
    {
        clearmouseclick(WM_LBUTTONDOWN);
        C.x = mousex();
        C.y = mousey();

        if(C.x >= X1 && C.x <= Y1 && C.y >= 200 && C.y <= 250 && pieseDisponibile(jucator, SMALL) == true)
        {
            marimePiesa = 1;
            selecteazaPiesaUnuiJucatorBackEnd(jucator, SMALL);
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
        }

        else if(C.x >= X2 && C.x <= Y2 && C.y >= 350 && C.y <= 400 && pieseDisponibile(jucator, MEDIUM) == true)
        {
            marimePiesa = 2;
            selecteazaPiesaUnuiJucatorBackEnd(jucator, MEDIUM);
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
        }

        else if(C.x >= X3 && C.x <= Y3 && C.y >= 500 && C.y <= 575 && pieseDisponibile(jucator, BIG) == true)
        {
            marimePiesa = 3;
            selecteazaPiesaUnuiJucatorBackEnd(jucator, BIG);
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
        }

        else if(C.x >= 300 && C.x <= 750 && C.y >= 150 && C.y <= 600 && piesaDePeTabla == false && piesaDinVarf(tabla[C.y/150-1][C.x/150-2]).culoare == jucator.culoare && tabla[C.y/150-1][C.x/150-2].totalPiese > 0)
        {
            linieSelectata = C.y/150-1;
            coloanaSelectata = C.x/150-2;
            piesaDePeTabla = true;
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
        }
    }
}

void punerePiesaJucatorFrontEnd(char* numeImagine, Jucator jucator)
{
    Piesa piesa;
    piesa.culoare = jucator.culoare;

    cout<<' ';
    if(ismouseclick(WM_LBUTTONDOWN) && marimePiesa != 0)
    {
        clearmouseclick(WM_LBUTTONDOWN);
        C.x = mousex();
        C.y = mousey();

        if(C.x >= 300 && C.x <= 750 && C.y >= 150 && C.y <= 600 && (piesaDinVarf(tabla[C.y/150-1][C.x/150-2]).dimensiune < marimePiesa-1 || tabla[C.y/150-1][C.x/150-2].totalPiese == 0))
        {
            if(marimePiesa == 3)
            {
                PlaySound(TEXT(sunet), NULL, SND_ASYNC);
                piesa.dimensiune = BIG;
                adaugaPiesaPePozitie(tabla[C.y/150-1][C.x/150-2], piesa);
                readimagefile(numeImagine, C.x/150*150+10, C.y/150*150+10, C.x/150*150+140, C.y/150*150+140);
                actualizeazaPieseDisponibileFrontEnd(piesa);
            }

            if(marimePiesa == 2)
            {
                PlaySound(TEXT(sunet), NULL, SND_ASYNC);
                piesa.dimensiune = MEDIUM;
                adaugaPiesaPePozitie(tabla[C.y/150-1][C.x/150-2], piesa);
                readimagefile(numeImagine, C.x/150*150+41, C.y/150*150+41, C.x/150*150+109, C.y/150*150+109);
                actualizeazaPieseDisponibileFrontEnd(piesa);
            }

            if(marimePiesa == 1)
            {
                PlaySound(TEXT(sunet), NULL, SND_ASYNC);
                piesa.dimensiune = SMALL;
                adaugaPiesaPePozitie(tabla[C.y/150-1][C.x/150-2], piesa);
                readimagefile(numeImagine, C.x/150*150+57, C.y/150*150+57, C.x/150*150+93, C.y/150*150+93);
                actualizeazaPieseDisponibileFrontEnd(piesa);
            }

            swap(jucatorCurent.culoare, jucatorUrmator.culoare);
            marimePiesa = 0;
            randPlayer = true;
        }
    }
    else if(ismouseclick(WM_LBUTTONDOWN) && piesaDePeTabla == true)
    {
        clearmouseclick(WM_LBUTTONDOWN);
        C.x = mousex();
        C.y = mousey();

        if(C.x >= 300 && C.x <= 750 && C.y >= 150 && C.y <= 600)
        {
            if(piesaDinVarf(tabla[linieSelectata][coloanaSelectata]).dimensiune > piesaDinVarf(tabla[C.y/150-1][C.x/150-2]).dimensiune || tabla[C.y/150-1][C.x/150-2].totalPiese == 0)
            {
                PlaySound(TEXT(sunet), NULL, SND_ASYNC);
                mutaPiesaDePeTablaBackEnd(linieSelectata, coloanaSelectata, C.y/150-1,C.x/150-2, jucator);
                mutaPiesaDePeTablaFrontEnd(C.x, C.y, numeImagine, piesaDinVarf(tabla[C.y/150-1][C.x/150-2]).dimensiune);

                swap(jucatorCurent.culoare, jucatorUrmator.culoare);
                piesaDePeTabla = false;
                randPlayer = true;
            }
        }
    }
}

void punerePiesaBot(Dimensiune dim, int lin, int col)
{

    Piesa blue;
    blue.culoare=ALBASTRU;
    blue.dimensiune = dim;

    adaugaPiesaPePozitie(tabla[lin][col], blue);
    C.y = 150*(lin+1);
    C.x = 150*(col+2);

    if(dim == BIG)
    {
        readimagefile("blue_fara_numar.jpg",C.x+10, C.y+10, C.x+140, C.y+140);
        actualizeazaPieseDisponibileFrontEnd(blue);
        selecteazaPiesaUnuiJucatorBackEnd(jucatorulAlbastru, dim);
    }
    else if(dim == MEDIUM)
    {
        readimagefile("blue_fara_numar.jpg",C.x+41, C.y+41, C.x+109, C.y+109);
        actualizeazaPieseDisponibileFrontEnd(blue);
        selecteazaPiesaUnuiJucatorBackEnd(jucatorulAlbastru, dim);
    }
    else if(dim == SMALL)
    {
        readimagefile("blue_fara_numar.jpg",C.x+57, C.y+57, C.x+93, C.y+93);
        actualizeazaPieseDisponibileFrontEnd(blue);
        selecteazaPiesaUnuiJucatorBackEnd(jucatorulAlbastru, dim);
    }
    randPlayer = true;
}

void numarDePiese()
{
    outtextxy(90, 220, "x2");
    outtextxy(90, 367, "x2");
    outtextxy(90, 528, "x2");
    outtextxy(990, 220, "x2");
    outtextxy(990, 367, "x2");
    outtextxy(990, 528, "x2");
}
void imagini()
{
    readimagefile("orange_fara_numar.jpg", 110, 210, 140, 250);
    readimagefile("orange_fara_numar.jpg", 110, 350, 170, 400);
    readimagefile("orange_fara_numar.jpg", 110, 500, 200, 575);
    readimagefile("blue_fara_numar.jpg", 950, 200, 990, 250);
    readimagefile("blue_fara_numar.jpg", 930, 350, 990, 400);
    readimagefile("blue_fara_numar.jpg",900, 500, 990, 575);
}

void tablaJoc()
{
    setfillstyle(SOLID_FILL, BLACK);
    bar(0,0,1100,790);
    line(450, 150, 450, 600);
    line(600, 150, 600, 600);
    line(300, 300, 750, 300);
    line(300, 450, 750, 450);
    rectangle(470, 695, 578, 720);
    outtextxy(474, 700, "Inapoi la meniu");
    rectangle(470, 30, 578, 55);
    outtextxy(495, 35, "Resetare");
}

void randJucator()
{

    if(jucatorCurent.culoare == PORTOCALIU && randPlayer == true)
    {
        bar(849, 624, 1100, 675);
        setcolor(COLOR(255, 140, 0));
        outtextxy(100, 625, "Este randul lui Portocaliu");
        setcolor(15);
        randPlayer = false;
    }
    else if(randPlayer == true)
    {
        bar(99, 624, 400, 675);
        setcolor(LIGHTBLUE);
        outtextxy(850, 625, "Este randul lui Albastru");
        setcolor(15);
        randPlayer = false;
    }
}

void finalJoc()
{
    apasatResetare = true;

    PlaySound(TEXT("victory.wav"), NULL, SND_ASYNC);
    if(cine_e_castigatorul() == PORTOCALIU)
        outtextxy(442, 90, "PORTOCALIU A CASTIGAT!");
    else
        outtextxy(447, 90, "ALBASTRU A CASTIGAT!");
}

void butonResetare()
{
    punct p;

    if(ismouseclick(WM_LBUTTONDOWN))
    {
        p.x = mousex();
        p.y = mousey();
        if(470 <= p.x && 30 <= p.y && p.x <= 578 && p.y <= 55)
        {
            tablaJoc();
            imagini();
            numarDePiese();
            reset();
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            apasatResetare = false;
            randPlayer = true;
        }
    }
}

void reguli()
{
    setfillstyle(SOLID_FILL, BLACK);
    bar(0, 0, 1100, 790);
    outtextxy(425, 50, "Bun venit la jocul Gobblet Gobblers");
    outtextxy(250, 150, "Jocul Gobblet Gobblers se bazeaza pe regulile jocului clasic de X si 0 insa se adauga un element");
    outtextxy(250, 170, "care il face extrem de atractiv, distractiv si nu foarte simplu de castigat dimpotriva.");
    outtextxy(250, 190, "Astfel, piesele au trei marimi: mare, mijlocie si mica si ele se pot manca una pe alta. ");
    outtextxy(250, 210, "La fel ca in X si 0, scopul este formarea unei linii de trei piese de aceeasi culoare. Insa, spre  ");
    outtextxy(250, 230, "deosebire de jocul clasic, piesele se pot aseza una peste alta, cea mijlocie peste cea mica, cea ");
    outtextxy(250, 250, "mare peste cele mijlocii sau mici.");
    outtextxy(250, 270, "Jucatorii primesc cate 6 piese de o culoare (2 piese mici, 2 piese mijlocii si 2 piese mari).");
    outtextxy(250, 290, "Pe rand fiecare jucator poate aseza o noua piesa intr-o pozitie libera sau pe o pozitie ocupata de");
    outtextxy(250, 310, "o piesa mai mica, sau poate muta o piesa pe o alta pozitie libera sau peste o alta piesa.");
    outtextxy(250, 330, "Pe parcurs piesele ajung sa se manance una pe alta sau sa iasa la iveala acolo unde nu te");
    outtextxy(250, 350, "astepti si sa iti strice planurile.");
    outtextxy(250, 370, "Gobblet Gobblers combina astfel strategia cu memoria, jucatorii fiind nevoiti sa tina minte piesele");
    outtextxy(250, 390, "ce sunt acoperite de altele mai mari, pentru a nu crea un avantaj jucatorului advers.");
    outtextxy(250, 410, "Jucatorii nu au voie sa sa se uite sub piese, iar o piesa atinsa trebuie mutata.");
    outtextxy(250, 430, "Jucatorul care reuseste primul sa formeze o linie de trei piese de aceeasi culoare este castigator.");
    outtextxy(250, 600, "Controale:");
    outtextxy(250, 620, "-apasati click-stanga pe piesa pe care doriti sa o selectati iar apoi click-stanga pe locul de pe tabla unde doriti sa o mutati");
    outtextxy(250, 640,"-apasati SETARI pentru a schimba modul de joc: PLAYER VS PLAYER sau PLAYER VS COMPUTER");
    rectangle(470, 695, 578, 720);
    outtextxy(474, 700, "Inapoi la meniu");
}

void initializeazaPaginaSetari()
{

    if(tipJoc == PLAYER_VS_PLAYER)
        setcolor(YELLOW);
    else
        setcolor(WHITE);

    rectangle(450, 130, 600, 180);
    outtextxy(475, 150, "Player vs Player");

    if(tipJoc == PLAYER_VS_EASY)
        setcolor(YELLOW);
    else
        setcolor(WHITE);

    rectangle(425, 300, 625, 350);
    outtextxy(440, 320, "Player vs Computer (Easy)");

    if(tipJoc == PLAYER_VS_HARD)
        setcolor(YELLOW);
    else
        setcolor(WHITE);

    rectangle(425, 500, 625, 550);
    outtextxy(440, 520, "Player vs Computer (Hard)");

    setcolor(WHITE);

}

void butoaneSetariFrontEnd()
{
    setfillstyle(SOLID_FILL, BLACK);
    bar(0, 0, 1100, 790);
    rectangle(150, 100, 175, 125);
    outtextxy(180, 105, "Sound Effects");
    rectangle(470, 695, 578, 720);
    outtextxy(474, 700, "Inapoi la meniu");

    if(strcmp(sunet, "click.wav") == 0)
        outtextxy(154, 104, "On");
    else
        outtextxy(154, 104, "Off");

    initializeazaPaginaSetari();
}

void butoaneSetariBackEnd()
{
    punct p;

    if(ismouseclick(WM_LBUTTONDOWN))
    {
        clearmouseclick(WM_LBUTTONDOWN);
        p.x = mousex();
        p.y = mousey();
        if(p.x >= 150 && p.x <= 175 && p.y >= 100 && p.y <= 125)
        {
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            swap(sunet, nimic);
            if(strcmp(sunet, "click.wav") == 0)
                outtextxy(154, 104, "On");
            else
                outtextxy(154, 104, "Off");
        }
        else if(470 <= p.x && 695 <= p.y && p.x <= 578 && p.y <= 720)
        {
            initializeazaNrPieseFrontEnd();
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            apasatInapoiLaMeniu = false;
            setfillstyle(SOLID_FILL, BLACK);
            bar(0, 0, 1100, 790);
            deseneazaMeniul();
            comanda = 0;
            initializeazaTabla();
            jucatorCurent.culoare = PORTOCALIU;
            jucatorUrmator.culoare = ALBASTRU;
        }
        else if(p.x >= 450 && p.x <= 600 && p.y >= 130 && p.y <= 180)
        {
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            tipJoc = PLAYER_VS_PLAYER;
            initializeazaPaginaSetari();
        }
        else if(p.x >= 425 && p.x <= 625 && p.y >= 300 && p.y <= 350)
        {
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            tipJoc = PLAYER_VS_EASY;
            initializeazaPaginaSetari();
        }
        else if(p.x >= 425 && p.x <= 625 && p.y >= 500 && p.y <= 550)
        {
            PlaySound(TEXT(sunet), NULL, SND_ASYNC);
            tipJoc = PLAYER_VS_HARD;
            initializeazaPaginaSetari();
        }

    }
}

bool potPuneCaBot(int i, int j, Dimensiune dim)
{
    return pieseDisponibile(jucatorulAlbastru, dim) && (tabla[i][j].totalPiese == 0 || piesaDinVarf(tabla[i][j]).dimensiune < dim);
}

void mutaEasy()
{
    do
    {
        Dimensiune dim = (Dimensiune) (rand()%3);

        if(pieseDisponibile(jucatorulAlbastru, dim))
            while(true)
            {
                int i = rand()%3;
                int j = rand()%3;
                if(potPuneCaBot(i, j, dim))
                {
                    punerePiesaBot(dim, i, j);
                    return ;
                }
            }
    }
    while(true);
}

int contorizezCateSuntPeOLinie(int linie, Culoare culoare)
{
    int contor = 0;

    for(int i = 0; i < 3; i++)
        if(piesaDinVarf(tabla[linie][i]).culoare == culoare)
            contor++;

    return contor;
}

int contorizezCateSuntPeOColoana(int coloana, Culoare culoare)
{
    int contor = 0;

    for(int i = 0; i < 3; i++)
        if(piesaDinVarf(tabla[i][coloana]).culoare == culoare)
            contor++;

    return contor;
}

int contorizezCateSuntPeDiagonalaPrincipala(Culoare culoare)
{
    int contor = 0;

    for(int i = 0; i < 3; i++)
        if(piesaDinVarf(tabla[i][i]).culoare == culoare)
            contor++;

    return contor;
}

int contorizezCateSuntPeDiagonalaSecundara(Culoare culoare)
{
    int contor = 0;

    for(int i = 0; i < 3; i++)
        if(piesaDinVarf(tabla[2-i][i]).culoare == culoare)
            contor++;

    return contor;
}

bool dacaPotCastiga()
{
    for(int i = 0; i < 3; i++)
    {
        int cntLin = contorizezCateSuntPeOLinie(i, ALBASTRU);

        if(cntLin == 2)
            for(int j = 0; j < 3; j++)
                if(piesaDinVarf(tabla[i][j]).culoare != ALBASTRU)
                    for(int dimensiune = 0; dimensiune < 2; dimensiune++)
                        if(potPuneCaBot(i, j, (Dimensiune) dimensiune))
                        {
                            punerePiesaBot((Dimensiune) dimensiune, i, j);
                            return true;
                        }

        int cntCol = contorizezCateSuntPeOColoana(i, ALBASTRU);

        if(cntCol == 2)
            for(int j = 0; j < 3; j++)
                if(piesaDinVarf(tabla[j][i]).culoare != ALBASTRU)
                    for(int dimensiune = 0; dimensiune < 2; dimensiune++)
                        if(potPuneCaBot(j, i, (Dimensiune) dimensiune))
                        {
                            punerePiesaBot((Dimensiune) dimensiune, j, i);
                            return true;
                        }
    }

    int cntDP = contorizezCateSuntPeDiagonalaPrincipala(ALBASTRU);

    if(cntDP == 2)
        for(int j = 0; j < 3; j++)
            if(piesaDinVarf(tabla[j][j]).culoare != ALBASTRU)
                for(int dimensiune = 0; dimensiune < 2; dimensiune++)
                    if(potPuneCaBot(j, j, (Dimensiune) dimensiune))
                    {
                        punerePiesaBot((Dimensiune) dimensiune, j, j);
                        return true;
                    }

    int cntDS = contorizezCateSuntPeDiagonalaSecundara(ALBASTRU);

    if(cntDS == 2)
        for(int j = 0; j < 3; j++)
            if(piesaDinVarf(tabla[2 - j][j]).culoare != ALBASTRU)
                for(int dimensiune = 0; dimensiune < 2; dimensiune++)
                    if(potPuneCaBot(2 - j, j, (Dimensiune) dimensiune))
                    {
                        punerePiesaBot((Dimensiune) dimensiune, 2 - j, j);
                        return true;
                    }
    return false;
}

bool dacaAdversarulPoateCastiga()
{
    for(int i = 0; i < 3; i++)
    {
        int cntLin = contorizezCateSuntPeOLinie(i, PORTOCALIU);

        if(cntLin == 2)
            for(int j = 0; j < 3; j++)
                if(piesaDinVarf(tabla[i][j]).culoare != PORTOCALIU)
                    for(int dimensiune = 2; dimensiune >= 0; dimensiune--)
                        if(potPuneCaBot(i, j, (Dimensiune) dimensiune))
                        {
                            punerePiesaBot((Dimensiune) dimensiune, i, j);
                            return true;
                        }

        int cntCol = contorizezCateSuntPeOColoana(i, PORTOCALIU);

        if(cntCol == 2)
            for(int j = 0; j < 3; j++)
                if(piesaDinVarf(tabla[j][i]).culoare != PORTOCALIU)
                    for(int dimensiune = 2; dimensiune >= 0; dimensiune--)
                        if(potPuneCaBot(j, i, (Dimensiune) dimensiune))
                        {
                            punerePiesaBot((Dimensiune) dimensiune, j, i);
                            return true;
                        }
    }

    int cntDP = contorizezCateSuntPeDiagonalaPrincipala(PORTOCALIU);

    if(cntDP == 2)
        for(int j = 0; j < 3; j++)
            if(piesaDinVarf(tabla[j][j]).culoare != PORTOCALIU)
                for(int dimensiune = 2; dimensiune >= 0; dimensiune--)
                    if(potPuneCaBot(j, j, (Dimensiune) dimensiune))
                    {
                        punerePiesaBot((Dimensiune) dimensiune, j, j);
                        return true;
                    }

    int cntDS = contorizezCateSuntPeDiagonalaSecundara(PORTOCALIU);

    if(cntDS == 2)
        for(int j = 0; j < 3; j++)
            if(piesaDinVarf(tabla[2-j][j]).culoare != PORTOCALIU)
                for(int dimensiune = 2; dimensiune >= 0; dimensiune--)
                    if(potPuneCaBot(2-j, j, (Dimensiune) dimensiune))
                    {
                        punerePiesaBot((Dimensiune) dimensiune, 2-j, j);
                        return true;
                    }
    return false;
}
void mutaHard()
{
    if(jucatorulAlbastru.pieseMari == 2 && jucatorulAlbastru.pieseMedii == 2 && jucatorulAlbastru.pieseMici == 2)
    {
        if(potPuneCaBot(1, 1, BIG))
            punerePiesaBot(BIG, 1, 1);
        else
            mutaEasy();
        return ;
    }

    if(dacaPotCastiga())
        return ;

    if(dacaAdversarulPoateCastiga())
        return ;

    mutaEasy();
}

void mutaCalculator()
{
    if(tipJoc == PLAYER_VS_EASY)
        mutaEasy();
    else
        mutaHard();

    swap(jucatorCurent.culoare, jucatorUrmator.culoare);
}


int main()
{
    srand(time(0));
    initializeazaCuloareJucatori();
    initializeazaTabla();
    initwindow(1100, 790, "Gobblet Gobblers", 150, 10);
    deseneazaMeniul();

    do
    {
        butonulApasat = butonAles();
        if(butonulApasat != 0)
            comanda = butonulApasat;

        if(comanda == 1)
        {
            randPlayer = true;
            tablaJoc();
            imagini();
            numarDePiese();

            do
            {
                apasatInapoiLaMeniu = true;
                butonResetare();
                inapoiLaMeniu();

                if(jucatorCurent.culoare == PORTOCALIU)
                {
                    randJucator();
                    selectarePiesaJucatorFrontEnd(jucatorulPortocaliu);
                    punerePiesaJucatorFrontEnd("orange_fara_numar.jpg", jucatorulPortocaliu);
                }
                else
                {
                    randJucator();
                    if(tipJoc == PLAYER_VS_PLAYER)
                    {
                        selectarePiesaJucatorFrontEnd(jucatorulAlbastru);
                        punerePiesaJucatorFrontEnd("blue_fara_numar.jpg", jucatorulAlbastru);
                    }
                    else
                    {
                        Sleep(1000);
                        mutaCalculator();
                    }
                }
            }
            while(cine_e_castigatorul() == NICIUNA && apasatInapoiLaMeniu == true);

            if(cine_e_castigatorul() != NICIUNA)
            {
                finalJoc();
                while(apasatInapoiLaMeniu == true && apasatResetare == true)
                {
                    inapoiLaMeniu();
                    butonResetare();
                }
            }
        }

        if(comanda == 2)
        {
            butoaneSetariFrontEnd();
            do
            {
                apasatInapoiLaMeniu = true;
                butoaneSetariBackEnd();
            }
            while(apasatInapoiLaMeniu == true);
        }

        if(comanda == 3)
        {
            reguli();
            do
            {
                cout<<' ';
                apasatInapoiLaMeniu = true;
                inapoiLaMeniu();
                clearmouseclick(WM_LBUTTONDOWN);
            }
            while(apasatInapoiLaMeniu == true);
        }
    }
    while(comanda != 4);
    closegraph();
    return 0;
}
