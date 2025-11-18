#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "Players_data.h"

#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LENGTH 50
#define MAX_TEAMS 10

typedef enum {
    ROLE_BATSMAN = 1,
    ROLE_BOWLER = 2,
    ROLE_ALLROUNDER = 3
} PlayerRole;

typedef struct PlayersList{
    unsigned short playerID;
    char playerName[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    PlayerRole playerRole;
    unsigned short totalRuns;
    float battingAverage;
    float strikeRate;
    unsigned short wickets;
    float economyRate;
    float performanceIndex;
    struct PlayersList *next;
} PlayersList;

typedef struct TeamList{
    unsigned short teamID;
    char teamName[MAX_NAME_LENGTH + 1];
    unsigned short totalPlayers;
    float averageBattingStrikeRate;
    unsigned short maxID;
    PlayersList *batsmenHead;
    PlayersList *bowlersHead;
    PlayersList *allRoundersHead;
    float totalBattingStrikeRate;
    int battingPlayersCount;
} TeamList;

typedef struct {
    TeamList teams[MAX_TEAMS];
} ICCSystem;

unsigned short getValidTeamID(){
    char buffer[50];
    unsigned short teamID = 0;
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    if (sscanf(buffer, "%hu", &teamID) != 1) {
        printf("Invalid input.\n");
        return 0;
    }
    if(teamID < 1 || teamID > MAX_TEAMS){
        return 0;
    }
    return teamID;
}

unsigned short getValidPlayerRole(){
    char buffer[50];
    unsigned short role = 0;
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-Rounder): ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    sscanf(buffer, "%hu", &role);
    if(role < 1 || role > 3){
        printf("Invalid role!\n");
        return 0;
    }
    return role;
}

float getValidFloat(){
    char buffer[100];
    float number;
    while(1){
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Invalid input. Please enter again: ");
            continue;
        }
        if (sscanf(buffer, "%f", &number) == 1) {
            return number;
        }
        printf("Invalid input. Please enter again: ");
    }
}

unsigned short getValidInteger(){
    char buffer[100];
    unsigned short number;
    while(1){
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Invalid input. Please enter again: ");
            continue;
        }
        if (sscanf(buffer, "%hu", &number) == 1) {
            return number;
        }
        printf("Invalid input. Please enter again: ");
    }
}

const char* getRoleString(PlayerRole role) {
    switch(role) {
        case ROLE_BATSMAN: return "Batsman";
        case ROLE_BOWLER: return "Bowler";
        case ROLE_ALLROUNDER: return "All-rounder";
        default: return "Unknown";
    }
}

PlayerRole getRoleFromString(const char *roleStr) {
    if (strcmp(roleStr, "Batsman") == 0) return ROLE_BATSMAN;
    if (strcmp(roleStr, "Bowler") == 0) return ROLE_BOWLER;
    return ROLE_ALLROUNDER;
}

PlayersList* getRoleListHead(TeamList *team, PlayerRole role) {
    if (role == ROLE_BATSMAN) return team->batsmenHead;
    if (role == ROLE_BOWLER) return team->bowlersHead;
    return team->allRoundersHead;
}

PlayersList** getRoleListHeadPtr(TeamList *team, PlayerRole role) {
    if (role == ROLE_BATSMAN) return &team->batsmenHead;
    if (role == ROLE_BOWLER) return &team->bowlersHead;
    return &team->allRoundersHead;
}

int isBattingRole(PlayerRole role) {
    return (role == ROLE_BATSMAN || role == ROLE_ALLROUNDER);
}

int findTeamIndexByName(ICCSystem *ICCAnalyzer, const char *teamName) {
    for (int i = 0; i < MAX_TEAMS; i++) {
        if (strcmp(ICCAnalyzer->teams[i].teamName, teamName) == 0) return i;
    }
    return -1;
}

int findTeamIndexByID(ICCSystem *ICCAnalyzer, unsigned short teamID){
    int low = 0;
    int high = MAX_TEAMS - 1;
    while(low <= high){
        int mid = low + (high - low) / 2;
        if(ICCAnalyzer->teams[mid].teamID == teamID) return mid;
        if(ICCAnalyzer->teams[mid].teamID < teamID) low = mid + 1;
        else high = mid - 1;
    } 
    return -1;
}

void updateAverageBattingSR(TeamList *team, float newStrikeRate) {
    team->totalBattingStrikeRate += newStrikeRate;
    team->battingPlayersCount++;
    
    team->averageBattingStrikeRate = team->totalBattingStrikeRate / team->battingPlayersCount;
}

float calculatePerformanceIndex(const PlayersList *player) {
    if (player->playerRole == ROLE_BATSMAN) {
        return (player->battingAverage * player->strikeRate) / 100.0f;
    }
    else if (player->playerRole == ROLE_BOWLER) {
        return (2.0f * player->wickets) + (100.0f - player->economyRate);
    }
    else { 
        float battingPI = (player->battingAverage * player->strikeRate) / 100.0f;
        float bowlingPI = player->wickets * 2.0f;
        return battingPI + bowlingPI;
    }
}

void insertPlayerSorted(PlayersList **head, PlayersList *newPlayer) {
    if (*head == NULL || (*head)->performanceIndex < newPlayer->performanceIndex) {
        newPlayer->next = *head;
        *head = newPlayer;
        return;
    }
    
    PlayersList *current = *head;
    while (current->next != NULL && current->next->performanceIndex >= newPlayer->performanceIndex) {
        current = current->next;
    }
    
    newPlayer->next = current->next;
    current->next = newPlayer;
}

int countListNodes(PlayersList *head) {
    int count = 0;
    while (head != NULL) {
        count++;
        head = head->next;
    }
    return count;
}

void freePlayersList(PlayersList *head) {
    while (head != NULL) {
        PlayersList *next = head->next;
        free(head);
        head = next;
    }
}

void printHeader(){
    printf("====================================================================================================\n");
    printf("%-5s %-20s %-15s %-8s %-8s %-8s %-8s %-8s %-8s\n", 
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("====================================================================================================\n");
}

void printPlayerData(PlayersList *player){
    printf("%-5hu %-20s %-15s %-8hu %-8.2f %-8.2f %-8hu %-8.2f %-8.2f\n",
           player->playerID, player->playerName, getRoleString(player->playerRole),
           player->totalRuns, player->battingAverage, player->strikeRate,
           player->wickets, player->economyRate, player->performanceIndex);
}

void printRolePlayersList(PlayersList *head) {
    while (head != NULL) {
        printPlayerData(head);
        head = head->next;
    }
}

void swapHeapElements(PlayersList *heap[], int teamIndex[], int i, int j) {
    PlayersList *tempPlayer = heap[i];
    heap[i] = heap[j];
    heap[j] = tempPlayer;
    
    int tempTeam = teamIndex[i];
    teamIndex[i] = teamIndex[j];
    teamIndex[j] = tempTeam;
}

void heapifyDown(PlayersList *heap[], int teamIndex[], int size, int i) {
    int largest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;

    if (left < size && heap[left]->performanceIndex > heap[largest]->performanceIndex)
        largest = left;

    if (right < size && heap[right]->performanceIndex > heap[largest]->performanceIndex)
        largest = right;

    if (largest != i) {
        swapHeapElements(heap, teamIndex, i, largest);
        heapifyDown(heap, teamIndex, size, largest);
    }
}

void heapifyUp(PlayersList *heap[], int teamIndex[], int i) {
    int parent = (i - 1) / 2;

    while (i > 0 && heap[i]->performanceIndex > heap[parent]->performanceIndex){   
        swapHeapElements(heap, teamIndex, i, parent);
        i = parent;
        parent = (i - 1) / 2;
    }
}

PlayersList* createPlayerNode(int playerIndex) {
    PlayersList *temp = malloc(sizeof(PlayersList));
    if(!temp) return NULL;
    
    temp->playerID = players[playerIndex].id;
    strncpy(temp->playerName, players[playerIndex].name, MAX_NAME_LENGTH);
    temp->playerName[MAX_NAME_LENGTH] = '\0';
    strncpy(temp->teamName, players[playerIndex].team, MAX_NAME_LENGTH);
    temp->teamName[MAX_NAME_LENGTH] = '\0';
    temp->playerRole = getRoleFromString(players[playerIndex].role);
    temp->totalRuns = players[playerIndex].totalRuns;
    temp->battingAverage = players[playerIndex].battingAverage;
    temp->strikeRate = players[playerIndex].strikeRate;
    temp->wickets = players[playerIndex].wickets;
    temp->economyRate = players[playerIndex].economyRate;
    temp->next = NULL;
    temp->performanceIndex = calculatePerformanceIndex(temp);
    
    return temp;
}

void addPlayerToTeam(TeamList *team, PlayersList *player) {
    team->totalPlayers++;
    if(player->playerID > team->maxID){
        team->maxID = player->playerID;
    }

    PlayersList **roleHead = getRoleListHeadPtr(team, player->playerRole);
    insertPlayerSorted(roleHead, player);
    
    if (isBattingRole(player->playerRole)) {
        updateAverageBattingSR(team, player->strikeRate);
    }
}

void inputPlayerName(char *name) {
    printf("Name: ");
    if (fgets(name, MAX_NAME_LENGTH + 1, stdin)) {
        name[strcspn(name, "\n")] = '\0';
    }
}

void inputPlayerStats(PlayersList *player) {
    printf("Total Runs: ");
    player->totalRuns = getValidInteger();

    printf("Batting Average: ");
    player->battingAverage = getValidFloat();

    printf("Strike Rate: ");
    player->strikeRate = getValidFloat();

    printf("Wickets: ");
    player->wickets = getValidInteger();

    printf("Economy Rate: ");
    player->economyRate = getValidFloat();
}

void initializeTeamsData(ICCSystem *ICCAnalyzer){
    for(int i = 0; i < teamCount; i++){
        TeamList *temp = &ICCAnalyzer->teams[i];
        temp->teamID = i + 1;
        strncpy(temp->teamName, teams[i], MAX_NAME_LENGTH);
        temp->teamName[MAX_NAME_LENGTH] = '\0';
        temp->totalPlayers = 0;
        temp->averageBattingStrikeRate = 0.0f;
        temp->maxID = 0;
        temp->batsmenHead = NULL;
        temp->bowlersHead = NULL;
        temp->allRoundersHead = NULL;
        temp->totalBattingStrikeRate = 0.0f;
        temp->battingPlayersCount = 0;
    }
}

void initializePlayersData(ICCSystem *ICCAnalyzer){
    for(int i = 0; i < playerCount; i++){
        PlayersList *temp = createPlayerNode(i);
        if(!temp){
            printf("Memory Allocation Failed!\n");
            return;
        }

        int teamIndex = findTeamIndexByName(ICCAnalyzer, temp->teamName);
        if(teamIndex == -1) {
            free(temp);
            continue;
        }

        addPlayerToTeam(&ICCAnalyzer->teams[teamIndex], temp);
    }
}

void displayPlayers(ICCSystem *ICCAnalyzer){
    printf("\nEnter Team ID: ");
    unsigned short teamID = getValidTeamID();
    if(teamID == 0){
        printf("Enter correct TeamID!\n");
        return;
    }
    
    int teamIndex = findTeamIndexByID(ICCAnalyzer, teamID);
    if(teamIndex == -1){ 
        printf("Team not found!\n");
        return;
    }

    TeamList *team = &ICCAnalyzer->teams[teamIndex];

    printf("\nPlayers of Team %s:\n", team->teamName);
    printHeader();

    printRolePlayersList(team->batsmenHead);
    printRolePlayersList(team->bowlersHead);
    printRolePlayersList(team->allRoundersHead);

    printf("====================================================================================================\n");
    printf("Total Players: %hu\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n\n", team->averageBattingStrikeRate);
}

void addNewPlayer(ICCSystem *ICCAnalyzer){
    printf("\nEnter Team ID to add player: ");
    unsigned short teamID = getValidTeamID();
    if(teamID == 0){
        printf("Enter correct TeamID!\n");
        return;
    }

    int teamIndex = findTeamIndexByID(ICCAnalyzer, teamID);
    if(teamIndex == -1){  
        printf("Team not found!\n");
        return;
    }

    TeamList *teamPtr = &ICCAnalyzer->teams[teamIndex];
    
    if (teamPtr->totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("Team is FULL.\n");
        return;
    }
    
    PlayersList *newPlayer = malloc(sizeof(PlayersList)); 
    if (!newPlayer) {
        printf("Memory allocation failed!\n");
        return;
    }
    newPlayer->next = NULL;
    
    printf("Enter Player Details:\n");

    newPlayer->playerID = teamPtr->maxID + 1;
    teamPtr->maxID = newPlayer->playerID;
    printf("Player ID: %hu\n", newPlayer->playerID);

    inputPlayerName(newPlayer->playerName);

    unsigned short role = getValidPlayerRole();
    if(role == 0){
        printf("Enter Valid Role!\n");
        free(newPlayer);
        return;
    }
    newPlayer->playerRole = (PlayerRole)role;

    strcpy(newPlayer->teamName, teamPtr->teamName);

    inputPlayerStats(newPlayer);

    newPlayer->performanceIndex = calculatePerformanceIndex(newPlayer);

    PlayersList **roleHead = getRoleListHeadPtr(teamPtr, newPlayer->playerRole);
    insertPlayerSorted(roleHead, newPlayer);
    teamPtr->totalPlayers++;
    
    if (isBattingRole(newPlayer->playerRole)) {
        updateAverageBattingSR(teamPtr, newPlayer->strikeRate);
    }

    printf("Player added successfully to Team %s with ID %hu!\n", teamPtr->teamName, newPlayer->playerID);
}

void displayAverageBattingSR(ICCSystem *ICCAnalyzer){
    TeamList *teamList[MAX_TEAMS];

    for (int i = 0; i < MAX_TEAMS; i++){
        teamList[i] = &ICCAnalyzer->teams[i];
    }

    // Bubble sort - descending order
    for(int i = 0; i < MAX_TEAMS - 1; i++){
        for(int j = 0; j < MAX_TEAMS - i - 1; j++){
            if (teamList[j]->averageBattingStrikeRate < teamList[j + 1]->averageBattingStrikeRate) {
                TeamList *temp = teamList[j];
                teamList[j] = teamList[j + 1];
                teamList[j + 1] = temp;
            }
        }
    }

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("%-5s %-20s %-12s %-15s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("=========================================================\n");
    
    for(int i = 0; i < MAX_TEAMS; i++){
        TeamList *data = teamList[i];
        printf("%-5hu %-20s %-12.2f %-15hu\n", 
               data->teamID, data->teamName, 
               data->averageBattingStrikeRate, data->totalPlayers);
    }
    printf("=========================================================\n\n");
}

void displayTopKPlayers(ICCSystem *ICCAnalyzer){
    printf("\nEnter Team ID: ");
    unsigned short teamID = getValidTeamID();
    if(teamID == 0){
        printf("Enter correct TeamID!\n");
        return;
    }

    int teamIndex = findTeamIndexByID(ICCAnalyzer, teamID);
    if(teamIndex == -1){
        printf("Team not found!\n");
        return;
    }

    TeamList *team = &ICCAnalyzer->teams[teamIndex];

    unsigned short role = getValidPlayerRole();
    if(role == 0){
        printf("Enter Valid Role!\n");
        return;
    }

    printf("Enter Number of players: ");
    unsigned short K = getValidInteger();

    PlayersList *roleListHead = getRoleListHead(team, (PlayerRole)role);
    int count = countListNodes(roleListHead);
    const char *roleName = getRoleString((PlayerRole)role);
    
    if(count == 0){
        printf("No players of this role in the team!\n");
        return;
    }
    
    if(K > count){
        printf("There are only %d %s in team %s. Showing all.\n", count, roleName, team->teamName);
        K = count;
    }
    
    printf("\nTop %hu %s of Team %s:\n", K, roleName, team->teamName);
    printHeader();
    
    PlayersList *current = roleListHead;
    for(int i = 0; i < K && current != NULL; i++) {
        printPlayerData(current);
        current = current->next;
    }
    printf("\n");
}

void displayAllPlayersByRole(ICCSystem *ICCAnalyzer){
    unsigned short role = getValidPlayerRole();
    if(role == 0){
        printf("Enter Valid Role!\n");
        return;
    }

    const char *roleName = getRoleString((PlayerRole)role);

    printf("\n%s of all teams:\n", roleName);
    printf("==========================================================================================================\n");
    printf("%-5s %-25s %-15s %-12s %-6s %-5s %-5s %-5s %-5s %-10s\n",
        "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("==========================================================================================================\n");

    // Max-heap of player pointers
    PlayersList *heap[MAX_TEAMS];
    int heapTeamIndex[MAX_TEAMS];
    PlayersList *teamPointers[MAX_TEAMS];
    int heapSize = 0;
    
    // Initialize team pointers and build initial heap - O(t logt)
    for (int i = 0; i < MAX_TEAMS; i++) {
        PlayersList *roleHead = getRoleListHead(&ICCAnalyzer->teams[i], (PlayerRole)role);
        teamPointers[i] = roleHead;
        
        if (roleHead != NULL) {
            heap[heapSize] = roleHead;
            heapTeamIndex[heapSize] = i;
            heapifyUp(heap, heapTeamIndex, heapSize);
            heapSize++;
        }
    }
    
    if (heapSize == 0) {
        printf("\nNo %s found in any team.\n", roleName);
        printf("==========================================================================================================\n\n");
        return;
    }
    
    // Extract max and replace with next from same team - O(N logt)
    while (heapSize > 0) {
        PlayersList *player = heap[0];
        int team = heapTeamIndex[0];
        
        printf("%-5hu %-25s %-15s %-12s %-6hu %-5.1f %-5.1f %-5hu %-5.1f %-10.2f\n",
               player->playerID, player->playerName, player->teamName, getRoleString(player->playerRole),
               player->totalRuns, player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate, player->performanceIndex);
        
        teamPointers[team] = teamPointers[team]->next;
        PlayersList *nextPlayer = teamPointers[team];
        
        if (nextPlayer != NULL) {
            heap[0] = nextPlayer;
            heapifyDown(heap, heapTeamIndex, heapSize, 0);
        } else {
            heap[0] = heap[heapSize - 1];
            heapTeamIndex[0] = heapTeamIndex[heapSize - 1];
            heapSize--;
            if (heapSize > 0) {
                heapifyDown(heap, heapTeamIndex, heapSize, 0);
            }
        }
    }
    
    printf("==========================================================================================================\n\n");
}

void cleanupMemory(ICCSystem *ICCAnalyzer){
    for (int i = 0; i < MAX_TEAMS; i++) {
        TeamList *team = &ICCAnalyzer->teams[i];
        freePlayersList(team->batsmenHead);
        freePlayersList(team->bowlersHead);
        freePlayersList(team->allRoundersHead);
    }
}

void displayMenu(ICCSystem *ICCAnalyzer){
    unsigned short choice;
    
    while(1){
        printf("==============================================================================\n");
        printf("                    ICC ODI Player Performance Analyzer\n");
        printf("==============================================================================\n");
        printf("1. Add Player to Team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams\n");
        printf("6. Exit\n");
        printf("==============================================================================\n");
        printf("Enter your choice: ");

        choice = getValidInteger();

        switch (choice) {
            case 1:
                addNewPlayer(ICCAnalyzer);
                break;
            case 2:
                displayPlayers(ICCAnalyzer);
                break;
            case 3:
                displayAverageBattingSR(ICCAnalyzer);
                break;
            case 4:
                displayTopKPlayers(ICCAnalyzer);
                break;
            case 5:
                displayAllPlayersByRole(ICCAnalyzer);
                break;
            case 6:
                cleanupMemory(ICCAnalyzer);  
                printf("\nThank you for using ICC ODI Player Performance Analyzer!\n");
                exit(0);
                break;
            default:
                printf("Invalid choice! Please enter 1-6.\n\n");
                break;
        }
    }
}

int main(){
    ICCSystem ICCAnalyzer;
    
    initializeTeamsData(&ICCAnalyzer);
    initializePlayersData(&ICCAnalyzer);
    
    displayMenu(&ICCAnalyzer);
    cleanupMemory(&ICCAnalyzer);
    
    return 0;
}