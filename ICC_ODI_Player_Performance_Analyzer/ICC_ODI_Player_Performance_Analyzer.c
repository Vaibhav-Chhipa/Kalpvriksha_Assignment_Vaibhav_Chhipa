#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "Players_data.h"

#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LENGTH 50
#define MAX_TEAMS 10

typedef struct PlayersList{
    unsigned short playerID;
    char playerName[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    char playerRole[MAX_NAME_LENGTH + 1];
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
    PlayersList **batsmen;
    int batsmenCount;
    PlayersList **bowlers;
    int bowlersCount;
    PlayersList **allRounders;
    int allRoundersCount;
} TeamList;

typedef struct {
    TeamList teams[MAX_TEAMS];
    PlayersList *players; 
} ICCSystem;

typedef struct {
    PlayersList *player;
    int teamIndex;
    int playerIndex;
} HeapNode;

void swap(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyDown(HeapNode heap[], int size, int i) {
    int largest = i;
    int left = 2*i + 1;
    int right = 2*i + 2;

    if (left < size && heap[left].player->performanceIndex > heap[largest].player->performanceIndex)
        largest = left;

    if (right < size && heap[right].player->performanceIndex > heap[largest].player->performanceIndex)
        largest = right;

    if (largest != i) {
        swap(&heap[i], &heap[largest]);
        heapifyDown(heap, size, largest);
    }
}

void heapifyUp(HeapNode heap[], int i) {
    int parent = (i - 1) / 2;

    while (i > 0 && heap[i].player->performanceIndex > heap[parent].player->performanceIndex){   
        swap(&heap[i], &heap[parent]);
        i = parent;
        parent = (i - 1) / 2;
    }
}

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

void calculateAverageBattingSR(ICCSystem *ICCAnalyzer) {
    for (int i = 0; i < MAX_TEAMS; ++i) {
        TeamList *teamsWithRole = &ICCAnalyzer->teams[i];
        float totalSR = 0.0f;
        int count = 0;
        for (int j = 0; j < teamsWithRole->batsmenCount; ++j) {
            totalSR += teamsWithRole->batsmen[j]->strikeRate;
            count++;
        }
        for (int j = 0; j < teamsWithRole->allRoundersCount; ++j) {
            totalSR += teamsWithRole->allRounders[j]->strikeRate;
            count++;
        }
        teamsWithRole->averageBattingStrikeRate = (count > 0) ? (totalSR / count) : 0.0f;
    }
}

float calculatePerformanceIndex(const PlayersList *player) {
    if (strcmp(player->playerRole, "Batsman") == 0) {
        return (player->battingAverage * player->strikeRate) / 100.0f;
    }
    else if (strcmp(player->playerRole, "Bowler") == 0) {
        return (2.0f * player->wickets) + (100.0f - player->economyRate);
    }
    else { 
        float battingPI =(player->battingAverage * player->strikeRate) / 100.0f;
        float bowlingPI = player->wickets * 2.0f;
        return battingPI + bowlingPI;
    }
}

void printPlayerData(PlayersList *player){
     printf("%-5hu %-20s %-15s %-8hu %-8.2f %-8.2f %-8hu %-8.2f %-8.2f\n",
               player->playerID, player->playerName, player->playerRole,
               player->totalRuns, player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate, player->performanceIndex);
}

int comparePlayersByPerfDesc(const void *a, const void *b) {
    PlayersList *p1 = *(PlayersList * const *)a;
    PlayersList *p2 = *(PlayersList * const *)b;
    if (p1->performanceIndex < p2->performanceIndex) return 1;
    if (p1->performanceIndex > p2->performanceIndex) return -1;
    return 0;
}

void initializePlayersData(ICCSystem *ICCAnalyzer){
    ICCAnalyzer->players = NULL;
    PlayersList *playersNode = NULL;
    
    for(int i = 0; i < playerCount; i++){
        PlayersList *temp = malloc(sizeof(PlayersList));
        if(!temp){
            printf("Memory Allocation Failed!\n");
            return;
        }
        temp->playerID = players[i].id;
        strncpy(temp->playerName, players[i].name, MAX_NAME_LENGTH);
        temp->playerName[MAX_NAME_LENGTH] = '\0';
        strncpy(temp->teamName, players[i].team, MAX_NAME_LENGTH);
        temp->teamName[MAX_NAME_LENGTH] = '\0';
        strncpy(temp->playerRole, players[i].role, MAX_NAME_LENGTH);
        temp->playerRole[MAX_NAME_LENGTH] = '\0';
        temp->totalRuns = players[i].totalRuns;
        temp->battingAverage = players[i].battingAverage;
        temp->strikeRate = players[i].strikeRate;
        temp->wickets = players[i].wickets;
        temp->economyRate = players[i].economyRate;
        temp->next = NULL;
        temp->performanceIndex = calculatePerformanceIndex(temp);
        
        if(ICCAnalyzer->players == NULL){
            ICCAnalyzer->players = temp;
            playersNode = temp;
        } else {
            playersNode->next = temp;
            playersNode = temp;
        }

        int teamIndex = findTeamIndexByName(ICCAnalyzer, temp->teamName);
        if(teamIndex == -1) continue;

        TeamList *team = &ICCAnalyzer->teams[teamIndex];

        team->totalPlayers++;
        if(temp->playerID > team->maxID){
            team->maxID = temp->playerID;
        }

        if (strcmp(temp->playerRole, "Batsman") == 0) {
            if (team->batsmenCount < MAX_PLAYERS_PER_TEAM){
                team->batsmen = realloc(team->batsmen, (team->batsmenCount + 1) * sizeof(PlayersList*));
                if (!team->batsmen) continue;  
                team->batsmen[team->batsmenCount++] = temp;
            }
        } else if (strcmp(temp->playerRole, "Bowler") == 0) {
            if (team->bowlersCount < MAX_PLAYERS_PER_TEAM){
                team->bowlers = realloc(team->bowlers, (team->bowlersCount + 1) * sizeof(PlayersList*));
                if (!team->bowlers) continue; 
                team->bowlers[team->bowlersCount++] = temp;
            }
        } else {
            if (team->allRoundersCount < MAX_PLAYERS_PER_TEAM){
                team->allRounders = realloc(team->allRounders, (team->allRoundersCount + 1) * sizeof(PlayersList*));
                if (!team->allRounders) continue; 
                team->allRounders[team->allRoundersCount++] = temp;
            }
        }
    }
    
    for (int i = 0; i < MAX_TEAMS; i++) {
        TeamList *teamsWithRole = &ICCAnalyzer->teams[i];
        if (teamsWithRole->batsmenCount > 0) 
            qsort(teamsWithRole->batsmen, teamsWithRole->batsmenCount, sizeof(PlayersList*), comparePlayersByPerfDesc);
        if (teamsWithRole->bowlersCount > 0) 
            qsort(teamsWithRole->bowlers, teamsWithRole->bowlersCount, sizeof(PlayersList*), comparePlayersByPerfDesc);
        if (teamsWithRole->allRoundersCount > 0) 
            qsort(teamsWithRole->allRounders, teamsWithRole->allRoundersCount, sizeof(PlayersList*), comparePlayersByPerfDesc);
    }
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
        temp->batsmen = NULL;
        temp->bowlers = NULL;
        temp->allRounders = NULL;
        temp->batsmenCount = 0;
        temp->bowlersCount = 0;
        temp->allRoundersCount = 0;
    }
}

void printHeader(){
    printf("====================================================================================================\n");
    printf("%-5s %-20s %-15s %-8s %-8s %-8s %-8s %-8s %-8s\n", 
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("====================================================================================================\n");
}

void displayPlayers(ICCSystem *ICCAnalyzer){
    unsigned short teamID = 0;

    printf("\nEnter Team ID: ");
    teamID = getValidTeamID();
    if(teamID == 0){
        printf("Enter correct TeamID!\n");
        return;
    }
    
    int teamIndex = findTeamIndexByID(ICCAnalyzer, teamID);
    if(teamIndex == -1){ 
        printf("Team not found!\n");
        return;
    }

    TeamList *temp = &ICCAnalyzer->teams[teamIndex];

    printf("\nPlayers of Team %s:\n", temp->teamName);
    printHeader();

    for (int i = 0; i < temp->batsmenCount; i++) {
        PlayersList *p = temp->batsmen[i];
        printPlayerData(p);
    }

    for (int i = 0; i < temp->bowlersCount; i++) {
        PlayersList *p = temp->bowlers[i];
        printPlayerData(p);
    }

    for (int i = 0; i < temp->allRoundersCount; i++) {
        PlayersList *p = temp->allRounders[i];
        printPlayerData(p);
    }

    printf("====================================================================================================\n");
    printf("Total Players: %hu\n", temp->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n\n", temp->averageBattingStrikeRate);
}

void addNewPlayer(ICCSystem *ICCAnalyzer){
    unsigned short teamID = 0; 
     
    printf("\nEnter Team ID to add player: ");
    teamID = getValidTeamID();
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

    printf("Name: ");
    if (!fgets(newPlayer->playerName, MAX_NAME_LENGTH + 1, stdin)) {
        free(newPlayer);
        return;
    }
    newPlayer->playerName[strcspn(newPlayer->playerName, "\n")] = '\0';

    unsigned short role = getValidPlayerRole();
    if(role == 0){
        printf("Enter Valid Role!\n");
        free(newPlayer);
        return;
    }
    if(role == 1){
        strcpy(newPlayer->playerRole, "Batsman");
    } else if(role == 2){
        strcpy(newPlayer->playerRole, "Bowler");
    } else {
        strcpy(newPlayer->playerRole, "All-rounder");
    }

    strcpy(newPlayer->teamName, teamPtr->teamName);

    printf("Total Runs: ");
    newPlayer->totalRuns = getValidInteger();

    printf("Batting Average: ");
    newPlayer->battingAverage = getValidFloat();

    printf("Strike Rate: ");
    newPlayer->strikeRate = getValidFloat();

    printf("Wickets: ");
    newPlayer->wickets = getValidInteger();

    printf("Economy Rate: ");
    newPlayer->economyRate = getValidFloat();

    newPlayer->performanceIndex = calculatePerformanceIndex(newPlayer);

    teamPtr->totalPlayers++;

    PlayersList ***roleArrayPtr = NULL;
    int *count = NULL;
    
    if (strcmp(newPlayer->playerRole, "Batsman") == 0) { 
        roleArrayPtr = &teamPtr->batsmen; 
        count = &teamPtr->batsmenCount;
    } else if (strcmp(newPlayer->playerRole, "Bowler") == 0) { 
        roleArrayPtr = &teamPtr->bowlers; 
        count = &teamPtr->bowlersCount; 
    } else { 
        roleArrayPtr = &teamPtr->allRounders; 
        count = &teamPtr->allRoundersCount; 
    }

    PlayersList **tempArray = realloc(*roleArrayPtr, (*count + 1) * sizeof(PlayersList*));
    if (!tempArray) {
        printf("Memory allocation failed! Player not added.\n");
        teamPtr->totalPlayers--;
        free(newPlayer);
        return;
    }
    *roleArrayPtr = tempArray;
    
    (*roleArrayPtr)[*count] = newPlayer;
    (*count)++;

    // Insertion sort to keep descending order
    for (int i = (*count) - 1; i > 0; i--) {
        if ((*roleArrayPtr)[i]->performanceIndex > (*roleArrayPtr)[i-1]->performanceIndex) {
            PlayersList *tempSwap = (*roleArrayPtr)[i-1]; 
            (*roleArrayPtr)[i-1] = (*roleArrayPtr)[i]; 
            (*roleArrayPtr)[i] = tempSwap;
        } else break;
    }

    // Add to main linked list
    if (ICCAnalyzer->players == NULL){
        ICCAnalyzer->players = newPlayer;
    } else {
        PlayersList *curr = ICCAnalyzer->players;
        while (curr->next != NULL)
            curr = curr->next;
        curr->next = newPlayer;
    }

    calculateAverageBattingSR(ICCAnalyzer);
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
        printf("%-5hu %-20s %-12.1f %-15hu\n", 
               data->teamID, data->teamName, 
               data->averageBattingStrikeRate, data->totalPlayers);
    }
    printf("=========================================================\n\n");
}

void displayTopKPlayers(ICCSystem *ICCAnalyzer){
    unsigned short teamID = 0;

    printf("\nEnter Team ID: ");
    teamID = getValidTeamID();
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

    PlayersList **rolePlayersList = NULL;
    int count = 0;
    char *roleName = "";
    
    if(role == 1) {
        rolePlayersList = team->batsmen;
        count = team->batsmenCount;
        roleName = "Batsmen";
    } else if(role == 2) {
        rolePlayersList = team->bowlers;
        count = team->bowlersCount;
        roleName = "Bowlers";
    } else if(role == 3) {
        rolePlayersList = team->allRounders;
        count = team->allRoundersCount;
        roleName = "All-rounders";
    } else {
        printf("Invalid role!\n");
        return;
    }
    
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
    
    // Array is already sorted, just print first K - O(K)
    for(int i = 0; i < K; i++) {
        PlayersList *player = rolePlayersList[i];
        printPlayerData(player);
    }
    printf("\n");
}

void displayAllPlayersByRole(ICCSystem *ICCAnalyzer){
    unsigned short role = getValidPlayerRole();
    if(role == 0){
        printf("Enter Valid Role!\n");
        return;
    }

    char *roleName;
    if(role == 1) roleName = "Batsmen";
    else if(role == 2) roleName = "Bowlers";
    else if(role == 3) roleName = "All-rounders";
    else {
        printf("Invalid Role!\n");
        return;
    }

    PlayersList **roleArr;
    int roleCount;

    HeapNode heap[MAX_TEAMS];
    int heapSize = 0;

    for (int i = 0; i < MAX_TEAMS; i++) {
        TeamList *team = &ICCAnalyzer->teams[i];

        if (team->totalPlayers == 0) continue;

        if (role == 1) {
            roleArr = team->batsmen;
            roleCount = team->batsmenCount;
        } else if (role == 2) {
            roleArr = team->bowlers;
            roleCount = team->bowlersCount;
        } else {
            roleArr = team->allRounders;
            roleCount = team->allRoundersCount;
        }

        if (roleCount == 0) continue;

        heap[heapSize].player = roleArr[0];
        heap[heapSize].teamIndex = i;
        heap[heapSize].playerIndex = 0;
        heapifyUp(heap, heapSize);
        heapSize++;
    }

    if (heapSize == 0) {
        printf("\nNo %s found in any team.\n\n", roleName);
        return;
    }

    printf("\n%s of all teams:\n", roleName);
    printf("==========================================================================================================\n");
    printf("%-5s %-25s %-15s %-12s %-6s %-5s %-5s %-5s %-5s %-10s\n",
        "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("==========================================================================================================\n");

    while (heapSize > 0) {
        HeapNode top = heap[0];
        PlayersList *player = top.player;

        printf("%-5hu %-25s %-15s %-12s %-6hu %-5.1f %-5.1f %-5hu %-5.1f %-10.2f\n",
               player->playerID, player->playerName, player->teamName, player->playerRole,
               player->totalRuns, player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate, player->performanceIndex);

        TeamList *team = &ICCAnalyzer->teams[top.teamIndex];

        if (role == 1) {
            roleArr = team->batsmen;
            roleCount = team->batsmenCount;
        } else if (role == 2) {
            roleArr = team->bowlers;
            roleCount = team->bowlersCount;
        } else {
            roleArr = team->allRounders;
            roleCount = team->allRoundersCount;
        }

        int nextIndex = top.playerIndex + 1;

        if (nextIndex < roleCount) {
            heap[0].player = roleArr[nextIndex];
            heap[0].playerIndex = nextIndex;
        }
        else {
            heap[0] = heap[--heapSize];
        }

        heapifyDown(heap, heapSize, 0);
    }
    printf("==========================================================================================================\n\n");
}

void cleanupMemory(ICCSystem *ICCAnalyzer){
    for (int i = 0; i < MAX_TEAMS; i++) {
        TeamList *team = &ICCAnalyzer->teams[i];
        if (team->batsmen) free(team->batsmen);
        if (team->bowlers) free(team->bowlers);
        if (team->allRounders) free(team->allRounders);
    }
    
    PlayersList *current = ICCAnalyzer->players;
    while (current != NULL) {
        PlayersList *next = current->next;
        free(current);
        current = next;
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
    ICCAnalyzer.players = NULL;
    
    initializeTeamsData(&ICCAnalyzer);
    initializePlayersData(&ICCAnalyzer);
    calculateAverageBattingSR(&ICCAnalyzer);
    
    displayMenu(&ICCAnalyzer);
    cleanupMemory(&ICCAnalyzer);
    
    return 0;
}