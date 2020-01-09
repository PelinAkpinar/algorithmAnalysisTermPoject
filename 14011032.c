#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 341813 //Aktorlerin tutuldugu hash tablosu uzunlugu
#define MAXLEN 255 //Kelime Uzunlugu
#define MAXBUFFER 10000 //Dosya okuma buffer boyutu
#define DELIMITER '/' //Dosya okuma belirteci
#define MAXACTORS 170600 //Maksimum aktor sayisi


//BFS icin kullanilacak durum belirtecleri
#define initial 1 
#define waiting 2
#define visited 3


char buffer[MAXBUFFER + 1];//Dosya okuma isleminde kullanilacak buffer

//Aktorleri tutan struct. Her bir aktor icin,
//Name: isim, ekrana yazdirmada kullanilir
//Key: ismin hashlenmis hali tekrar hesaplanmamak icin kullanilir
//hashIndex hash tablosunda tutuldugu indis. Program icerisinde aktor aramada kullanilir. Bulunan aktorun indisine gore komsuluk matrisi kurulacaktir
//ParentActor - Movie; BFS yaparken backtrack islemi icin kullanilir
//Bacon Kevin bacon icin bacon sayisi, diger arama icin agac seviyesi tutulur. BFS yapilirken ilklendirilip doldurulur.
struct Actor
{
    char name[MAXLEN];
    int key;
    int hashIndex;
    char parentMovie[MAXLEN];
    char parentActor[MAXLEN];
    int bacon;
};
//BFS algoritmasi icin global queue veri yapisi ilklendirilmesi
int queue[SIZE], front = -1, rear = -1;
int state[SIZE];//her bir aktorun graf icerisinde ziyaret edilip edilmedigi bilgisini tasiyan dizi
int movieState[SIZE];//her bir filmin ziyaret edilip edilmedigi bilgisini tutan dizi
void insert_queue(int);//Queue veri yapisina veri ekleyen fonksiyon
int delete_queue();//Queue veri yapisindan FIFO mantigiyla veri cikartan fonksiyon. Output olarak kuyruktaki ilk elemani verir ve kuyruktan siler
int isEmpty_queue();//Kuyrugun bos olup olmadigini kontrol eden fonksiyon
struct Actor* hashTable[SIZE];//Aktorlerin tutuldugu hash table. Boyutu, toplam beklenen aktor sayisi *2 den buyuk en buyuk asal sayi olarak secildi
struct Actor* theActor;//arama islemlerinde gecici eleman tutmamiza yarayan degisken
/*
	READFILE
	Dosya okuma islemi yapar ve komsuluk matrisini doldurur. Ardindan menuyu baslatan fonksiyonu cagirir.
	@param1 char* : Okunacak dosya ismi
	@param2 int**: 2 boyutlu komsuluk matrisinin baslangic adresi
	@param3 char**: Film listesinin tutuldugu dizi
*/
void readFile(char* , int** , char** );

/*
	Addtomovies
	Verilen filmi, movies dizisine atar ve bunu dinamik hafiza ile yapar
	@param1 char* listeye eklenecek filmin pointeri
	@param2 char** film listesinin pointeri
	@param3 int simdiye kadar sayilan film sayisi. Dinamik hafizanin (re)allocate edilmesinde kullanilir
*/
char** addToMovies(char* , char** , int );
/*
	Start
	Dosyalar okunup komsuluk matrisine eklendikten sonra, cagirilir. Sonsuz bir dongu icerisinde menuyu ekrana basar ve 
	kullanici girisine gore bacon veye iki aktor arasi mesefeyi bulur
	@param1 int** komusluk matrisi, her satir birer film her sutun birer aktor olaak sekilde bi partite graf 
	@param2 char** : filmlerin tutuldugu dizi
	@param3 int : toplam film sayisi
	@param4 int : unique aktor sayisi
*/
void start(int** , char** , int , int );
/*
	Bacon
	BFS islemleri icin ilklendirme yapar. Iceride butun nodelarin durumu 0 ve butun queue bosaltilr. Ayrica struct icinde bacon sayisi kalmissa onlar 0lanir
	@param1 int** : komsuluk matrisi
	@param2 char** : filmlerin tutuldugu liste
	@param3 int : film sayisi
	@param4 int : Unique aktor sayisi
	@param5 - @param6 char * , baslangic ve bitis aktor isimleri
*/
void bacon(int** , char** , int , int , char* , char* );
/*
	Bfs
	Bfs islemleri icin gerekli veriler sifirlandiktan sonra, graf dolasma islemini yapar.
	@param1 : komsuluk matrisi
	@param2 char** : film listesi
	@param3 int: film sayisi
	@param4 int: baslangic dugumunun indisi
	@param5 int: bitis dugumunun indisi
*/
void bfs(int** , char** , int , int , int );
/*
	getkey
	verilen stringi hashler ve geriye output olarak hashlenmis sayi degerini verir
	@param1 char* : hashlenecek string
*/
unsigned long getKey(char* );
/*
	insert
	hash listesine verilen keye gore eleman ekler.
	@param1 unsigned long : key
	@param2 char* : veri, bu senaryoda oyuncunun ismi
*/
int insert(unsigned long , char* );
/*
	verilen key degerine gore hash tablosundan aktor bulur
	@param1 : key

*/
struct Actor* search(unsigned long );
//hashcode = hashlenmis degerin tablo uzunlugune gore modu
int hashCode(unsigned long key);

int main()
{
    char* input = "input-mpaa.txt";
    char** movies = NULL;
    int** adjMatrix = NULL;
    readFile(input, adjMatrix, movies);
    return 0;
}

void readFile(char* input, int** adjMatrix, char** movies)
{
    char *p, *actor, *movie; //p dosya okuma islemlerinde kullanilir
    FILE* file;
    int len;//satir boyunu tutmak icin kullanilir
    int position;//hash tablosuna eklenecek aktorun veya bulunan aktorun pozisyonunu tutar
    int numberOfMovies = 0;
    int numberOfActors = 0;
    int i, j = 0;
    // Dosyalari okumak icin ac
    file = fopen(input, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening %s\n", input);
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        //satir satir oku, satir sonunda varsa /n karakterini sil
        len = strlen(buffer);
        buffer[len - 1] = DELIMITER;
        //Windows dosya okuma sorunu icin
        if (len > 1 && buffer[len - 2] == '\r')
        {
            buffer[len - 2] = DELIMITER;
            buffer[len - 1] = '\0';
        }
        //her satir film ismi ile baslar
        p = movie = buffer;
        p = strchr(p, DELIMITER); // pointeri filmin isminden sonraya tasi
        *p++ = '\0';//null karakteri ile degistir ve pointeri ilerlet
        numberOfMovies++;//film okundu sayisini arttir
        movies = addToMovies(movie, movies, numberOfMovies);//filmi listeye ekle
        adjMatrix = realloc(adjMatrix, sizeof *adjMatrix * numberOfMovies);//Her film icin yeni bir satir olustur
        adjMatrix[numberOfMovies - 1] = calloc(SIZE, sizeof(int));//Her satira yeteri kadar genislikte memory ata
        // Aktorleri oku
        actor = p;
        while ((p = strchr(p, DELIMITER)) != NULL)
        {
        	//her \ isareeti gordukten sonra null karakter koy
            *p++ = '\0';
            theActor = search(getKey(actor));//aktoru isme gore ara
            if (theActor != NULL)//eger varsa
            {
                position = theActor->hashIndex;//aktorun film ile baglantisini kurmak icin indisini al
            }
            else//eger yoksa
            {
                numberOfActors++; // hash table eklemek icin unique aktor sayisini arttir
                position = insert(getKey(actor), actor);//aktoru hash tablosuna ekle
            }
            adjMatrix[numberOfMovies - 1][position] = 1;//aktor ve filmm iliskisini komsuluk matrisinde belirt
            actor = p;//bir sonkari aktor icin devam et
        }
        putchar('\n');
        printf("Movies : %d Actors : %d\n", numberOfMovies, numberOfActors);
        //getchar();
    }
    //butun aktorler ve filmler okunup komsuluk matrisine atildiktan sonra;
    start(adjMatrix, movies, numberOfMovies, numberOfActors);
    return adjMatrix;
}

void start(int** matrix, char** movies, int numberOfMovies, int numberOfActors)
{
    int choice;//kullanici secimi 
    int i;
    char startWord[256];
    char stopWord[256];
    // pm(matrix,numberOfMovies,numberOfActors);
    do
    {
        printf("Menu\n\n");
        printf("1. Bacon number\n");
        printf("2. Distance btw two actors\n");
        printf("3. Exit\n");
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                printf("Enter actor name for bacon search\n");
                gets(stopWord);//ilk anlamsiz karakterden kurtulmak icin oku
                gets(stopWord);//burada baslangc kevin bacon oldugu icin hardcoded yazilmistir
                bacon(matrix, movies, numberOfMovies, numberOfActors, "Bacon, Kevin", stopWord);
                break;
            case 2:
                printf("Enter first Actor\n");
                gets(startWord);//ilk anlamsiz karakterden kurtulmak icin oku
                gets(startWord);
                printf("Enter second Actor\n");
                gets(stopWord);//ikinci aktoru oku
                bacon(matrix, movies, numberOfMovies, numberOfActors, startWord, stopWord);
                break;
            case 3:
                printf("Bye\n");
                freeMemory(movies,numberOfMovies,matrix);//memoryi serbest brak
                break;
            default:
                printf("Invalid choice!\n");
                break;
        }

    } while (choice != 3);
}
void bacon(
    int** matrix, char** movies, int numberOfMovies, int numberOfActors, char* start, char* stop)
{
    int v;//baslangic indisi icin 
    int p;//bitis indisi icin
    front = -1;//queue sifirlandi
    rear = -1;

    for (v = 0; v < SIZE; v++) //grafta kullanilan butun durum ve queue sifirlandi
    {
        state[v] = initial;
        movieState[v] = initial;
        queue[v] = 0;
    }
    theActor = search(getKey(start));//ilk inputu ara
    if (theActor != NULL)
    {
        printf("Actor found\n");
        v = theActor->hashIndex;//eger bulduysam indexini starta ata
    }
    else
    {
        printf("Actor not found : %s", start);//Bulamadiysam debug icin ismini ve hash degerini yaz
        printf("%d", getKey(start));
        return;
    }
    theActor = search(getKey(stop));//ikinci inputu ara ve ayni islemleri tekrarla
    if (theActor != NULL)
    {
        printf("Actor found\n");
        p = theActor->hashIndex;
    }
    else
    {
        printf("Actor not found : %s", stop);
        printf("%d", getKey(stop));
        return;
    }

    bfs(matrix, movies, numberOfMovies, v, p);//grafi gezmeye basla
}
void bfs(int** matrix, char** movies, int numberOfMovies, int v, int p)
{
    int i, j, k;//gecici indis tutmak icin degerler
    int init = v;//baslangic dugumu

    hashTable[v]->bacon = 0;//baslangic dugumunun seviyesi 0lanir
    insert_queue(v);//kuyruga eklenir
    state[v] = waiting;//durumu bekleniyor olarak degistirilir
    int stop = p; // bitis dugumu atanir
    int baconNo = 0;//toplam seviyeyi tutan degisken
    while (!isEmpty_queue())//kuyruk bos olmadigi surece
    {
        v = delete_queue();//kuyruktaki ilk elemani al
        printf("%s ", hashTable[v]->name);//ekrana yaz
        if (hashTable[v]->bacon >= 6)//eger 6. seviye ise bacon iliskisi yoktur
        {
            printf("Graph exceeds six");//debug icin ekrana bilgi ver
            return;//
        }
        state[v] = visited;//o node durumunu ziyaret edildi olarak isaretle
        if (v == stop)//eger aradigimiz deger ise ekrana bilgi ver
        {
            printf("\nGraph Level(Bacon no) : %d\n", hashTable[v]->bacon);
            while (v != init)//her bir parentActor icin backtrace yaparak baslangic noktasina kadar bilgi ver
            {
                printf("%s is played with %s in Movie :%s\n", hashTable[v]->name,
                    hashTable[v]->parentActor, hashTable[v]->parentMovie);
                theActor = search(getKey(hashTable[v]->parentActor));
                v = theActor->hashIndex;
            }
            return;
        }
        
        //Arama baslangici
        for (i = 0; i < numberOfMovies; i++)// her bir film icin
        {
            if (matrix[i][v] == 1 && movieState[i] == initial)//eger aktor o filmde oynamis ise ve o filmin elemanlari daha once kuyruga eklenmemisse
            {
                for (j = 0; j < SIZE; j++)//o filmdeki her bir oyuncu icin
                {
                    if (matrix[i][j] == 1 && state[j] == initial)//eger birlikte oynandiysa ve daha once kuyruga eklenmediyse
                    {
                        insert_queue(j);//o oyuncuyu kuyruga ekle
                        strcpy(hashTable[j]->parentMovie, movies[i]);//parent olarak looplarin icine girerken kullanilan aktoru ve filmi yaz
                        strcpy(hashTable[j]->parentActor, hashTable[v]->name);
                        hashTable[j]->bacon = hashTable[v]->bacon + 1;//bacon seviyesini bir arttir. Bacon seviyesi bir onceki aktorun bir fazlasi olarak arttirilir
                        state[j] = waiting;//durumunu kuyruga atilmis olarak isaretle
                    }
                }
                movieState[i] = visited;//filmi ziyaret edildi olarak isaretle
            }
        }
    }//bu islemleri kuyruk bitene kadar surdur
}

void insert_queue(int vertex)
{
    if (rear == SIZE - 1)
        printf("Queue Overflow\n");
    else
    {
        if (front == -1)
            front = 0;
        rear = rear + 1;
        queue[rear] = vertex;
    }
}

int isEmpty_queue()
{
    if (front == -1 || front > rear)
        return 1;
    else
        return 0;
}

int delete_queue()
{
    int delete_item;
    if (front == -1 || front > rear)
    {
        printf("Queue Underflow\n");
        exit(1);
    }

    delete_item = queue[front];
    front = front + 1;
    return delete_item;
}
void freeMemory(char** movies, int numberOfMovies, int** adjMatrix)
{
    int i;
    for (i = 0; i < numberOfMovies; i++)
    {
        free(movies[i]);
        free(adjMatrix[i]);
    }
    free(movies);
    free(adjMatrix);
}

char** addToMovies(char* movie, char** movies, int numberOfMovies)
{
    movies = realloc(movies, sizeof(*movies) * numberOfMovies);
    movies[numberOfMovies - 1] = malloc(255);
    strcpy(movies[numberOfMovies - 1], movie);
    return movies;
}

int hashCode(unsigned long key)
{
    return key % SIZE;
}
struct Actor* search(unsigned long key)
{
    //hash degerinin modunu alarak indexi bul
    int hashIndex = hashCode(key);

	//dizide aktoru keyini bulana kadar
    while (hashTable[hashIndex] != NULL)
    {
        if (hashTable[hashIndex]->key == key)
        {
            return hashTable[hashIndex];
        }

        
        ++hashIndex;
		//Tabloyu  son elemanlardan basa dogru don
        hashIndex %= SIZE;
    }

    return NULL;
}
int insert(unsigned long key, char* data)
{
    struct Actor* item = (struct Actor*)malloc(sizeof(struct Actor));
    strcpy(item->name, data);
    item->key = key;
    int hashIndex = hashCode(key);
    while (hashTable[hashIndex] != NULL)
    {
        ++hashIndex;
        hashIndex %= SIZE;
    }
    item->hashIndex = hashIndex;
    hashTable[hashIndex] = item;
    return hashIndex;
}
unsigned long getKey(char* str)
{
    unsigned long i, key;
    key = 0;
    i = 0;
    for (i = 0; i < strlen(str); i++)
    {
        key += round(str[i] * pow(3, strlen(str) - i - 1));
    }
    return key;
}
