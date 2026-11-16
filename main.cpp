// Music Library - Storage & Indexing Layer
// Classes: Song, SongHashTable (+HashNode), SkipNode, SkipList, MusicLibrary
// Only uses: iostream, fstream, string, cstdlib, ctime  (no STL containers)

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;

// ====================================================================
// 1. SONG
// ====================================================================
class Song {
private:
    int songID;
    string title;
    string artist;
    string album;
    int duration;          // in seconds

public:
    Song* next;            // used by MusicLibrary's linked list (songsHead)

    Song(int id = 0, string t = "", string ar = "", string al = "", int d = 0) {
        songID = id;
        title = t;
        artist = ar;
        album = al;
        duration = d;
        next = NULL;
    }

    int getID() { return songID; }
    string getTitle() { return title; }
    string getArtist() { return artist; }
    string getAlbum() { return album; }
    int getDuration() { return duration; }

    void display() {
        cout << "[" << songID << "] " << title << " | " << artist
             << " | " << album << " | " << duration / 60 << ":"
             << (duration % 60 < 10 ? "0" : "") << duration % 60 << endl;
    }
};

// ====================================================================
// 2. SONG HASH TABLE  (separate chaining, searches by song ID)
// ====================================================================
struct HashNode {
    Song* song;
    HashNode* next;
    HashNode(Song* s) {
        song = s;
        next = NULL;
    }
};

class SongHashTable {
private:
    HashNode** buckets;
    int capacity;
    int size;

public:
    SongHashTable(int cap = 1009) {          // 1009 is prime, good for 1000+ songs
        capacity = cap;
        size = 0;
        buckets = new HashNode*[capacity];
        for (int i = 0; i < capacity; i++)
            buckets[i] = NULL;
    }

    ~SongHashTable() {                        // frees nodes only (library owns songs)
        for (int i = 0; i < capacity; i++) {
            HashNode* cur = buckets[i];
            while (cur != NULL) {
                HashNode* temp = cur;
                cur = cur->next;
                delete temp;
            }
        }
        delete[] buckets;
    }

    int hashFunction(int id) {
        return id % capacity;
    }

    void insert(Song* s) {
        int index = hashFunction(s->getID());
        HashNode* node = new HashNode(s);
        node->next = buckets[index];          // insert at front of chain
        buckets[index] = node;
        size++;
    }

    Song* search(int id) {
        HashNode* cur = buckets[hashFunction(id)];
        while (cur != NULL) {
            if (cur->song->getID() == id)
                return cur->song;
            cur = cur->next;
        }
        return NULL;
    }

    bool remove(int id) {
        int index = hashFunction(id);
        HashNode* cur = buckets[index];
        HashNode* prev = NULL;
        while (cur != NULL) {
            if (cur->song->getID() == id) {
                if (prev == NULL)
                    buckets[index] = cur->next;
                else
                    prev->next = cur->next;
                delete cur;
                size--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }
};

// ====================================================================
// 3. SKIP NODE
// ====================================================================
class SkipNode {
public:
    Song* song;
    SkipNode** forward;     // array of next pointers, one per level
    int nodeLevel;          // number of levels this node has

    SkipNode(Song* s, int level) {
        song = s;
        nodeLevel = level;
        forward = new SkipNode*[level];
        for (int i = 0; i < level; i++)
            forward[i] = NULL;
    }

    ~SkipNode() {
        delete[] forward;
    }

    SkipNode* nextAtLevel(int level) {
        if (level < nodeLevel)
            return forward[level];
        return NULL;
    }
};

// ====================================================================
// 4. SKIP LIST  (keeps songs sorted by title)
// ====================================================================
class SkipList {
private:
    static const int MAX_LEVEL = 16;
    SkipNode* head;
    int maxLevel;
    int currentLevel;       // highest level index currently in use (0 = bottom)

public:
    SkipList() {
        maxLevel = MAX_LEVEL;
        currentLevel = 0;
        head = new SkipNode(NULL, maxLevel);
        srand(time(0));
    }

    ~SkipList() {                             // frees nodes only (library owns songs)
        SkipNode* cur = head;
        while (cur != NULL) {
            SkipNode* temp = cur;
            cur = cur->forward[0];
            delete temp;
        }
    }

    // coin flip: each extra level has 50% chance
    int randomLevel() {
        int level = 0;
        while (rand() % 2 == 1 && level < maxLevel - 1)
            level++;
        return level;
    }

    void insert(Song* s) {
        SkipNode* update[MAX_LEVEL];
        SkipNode* cur = head;
        string title = s->getTitle();

        for (int i = currentLevel; i >= 0; i--) {
            while (cur->nextAtLevel(i) != NULL &&
                   cur->forward[i]->song->getTitle() < title)
                cur = cur->forward[i];
            update[i] = cur;
        }

        int level = randomLevel();
        if (level > currentLevel) {
            for (int i = currentLevel + 1; i <= level; i++)
                update[i] = head;
            currentLevel = level;
        }

        SkipNode* node = new SkipNode(s, level + 1);
        for (int i = 0; i <= level; i++) {
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
    }

    // exact title search (case-sensitive)
    Song* search(string title) {
        SkipNode* cur = head;
        for (int i = currentLevel; i >= 0; i--) {
            while (cur->nextAtLevel(i) != NULL &&
                   cur->forward[i]->song->getTitle() < title)
                cur = cur->forward[i];
        }
        cur = cur->forward[0];
        if (cur != NULL && cur->song->getTitle() == title)
            return cur->song;
        return NULL;
    }

    // print every song whose title starts with prefix
    void listByPrefix(string prefix) {
        SkipNode* cur = head;
        for (int i = currentLevel; i >= 0; i--) {
            while (cur->nextAtLevel(i) != NULL &&
                   cur->forward[i]->song->getTitle() < prefix)
                cur = cur->forward[i];
        }
        cur = cur->forward[0];

        int found = 0;
        while (cur != NULL &&
               cur->song->getTitle().substr(0, prefix.length()) == prefix) {
            cur->song->display();
            found++;
            cur = cur->forward[0];
        }
        if (found == 0)
            cout << "No songs start with \"" << prefix << "\"" << endl;
    }
};

// ====================================================================
// 5. MUSIC LIBRARY  (main entry point)
// ====================================================================
class MusicLibrary {
private:
    Song* songsHead;
    int songCount;
    SongHashTable idIndex;
    SkipList titleIndex;
    string dataFile;

public:
    MusicLibrary(string file = "songs.txt") {
        songsHead = NULL;
        songCount = 0;
        dataFile = file;
    }

    ~MusicLibrary() {                         // library owns all Song objects
        while (songsHead != NULL) {
            Song* temp = songsHead;
            songsHead = songsHead->next;
            delete temp;
        }
    }

    bool addSong(Song* s) {
        if (idIndex.search(s->getID()) != NULL) {
            cout << "Song ID " << s->getID() << " already exists." << endl;
            return false;
        }
        s->next = songsHead;                  // add to main list
        songsHead = s;
        idIndex.insert(s);                    // fast search by ID
        titleIndex.insert(s);                 // sorted search by title
        songCount++;
        return true;
    }

    Song* findByID(int id) {
        return idIndex.search(id);
    }

    Song* searchTitle(string title) {
        return titleIndex.search(title);
    }

    void listByPrefix(string prefix) {
        titleIndex.listByPrefix(prefix);
    }

    int getCount() { return songCount; }

    // File format (one song per line):  id|title|artist|album|duration
    void saveToFile() {
        ofstream out(dataFile.c_str());
        if (!out) {
            cout << "Cannot open file for writing." << endl;
            return;
        }
        Song* cur = songsHead;
        while (cur != NULL) {
            out << cur->getID() << "|" << cur->getTitle() << "|"
                << cur->getArtist() << "|" << cur->getAlbum() << "|"
                << cur->getDuration() << endl;
            cur = cur->next;
        }
        out.close();
    }

    void loadFromFile() {
        ifstream in(dataFile.c_str());
        if (!in) {
            cout << "No data file found." << endl;
            return;
        }
        string line;
        while (getline(in, line)) {
            if (line == "") continue;
            string parts[5];
            int start = 0;
            for (int i = 0; i < 5; i++) {
                int pos = line.find('|', start);
                if (pos == (int)string::npos) pos = line.length();
                parts[i] = line.substr(start, pos - start);
                start = pos + 1;
            }
            addSong(new Song(atoi(parts[0].c_str()), parts[1], parts[2],
                             parts[3], atoi(parts[4].c_str())));
        }
        in.close();
    }
};

// ====================================================================
// Simple test
// ====================================================================
int main() {
    MusicLibrary lib("songs.txt");

    lib.addSong(new Song(1, "Blinding Lights", "The Weeknd", "After Hours", 200));
    lib.addSong(new Song(2, "Believer", "Imagine Dragons", "Evolve", 204));
    lib.addSong(new Song(3, "Shape of You", "Ed Sheeran", "Divide", 233));
    lib.addSong(new Song(4, "Bad Guy", "Billie Eilish", "WWAFAWDWG", 194));
    lib.addSong(new Song(5, "Perfect", "Ed Sheeran", "Divide", 263));
    lib.addSong(new Song(3, "Duplicate ID", "Test", "Test", 100));   // rejected

    cout << "Total songs: " << lib.getCount() << endl;

    cout << "\nFind by ID 3:" << endl;
    Song* s = lib.findByID(3);
    if (s != NULL) s->display();

    cout << "\nSearch title \"Perfect\":" << endl;
    s = lib.searchTitle("Perfect");
    if (s != NULL) s->display(); else cout << "Not found" << endl;

    cout << "\nSongs starting with \"B\":" << endl;
    lib.listByPrefix("B");

    lib.saveToFile();
    cout << "\nSaved to songs.txt" << endl;
    return 0;
}