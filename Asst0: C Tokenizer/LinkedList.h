typedef struct _Node_ {
	char* data;
	struct _Node_ * next;
} Node;

Node* addNode(Node*front, char*data);

Node* deleteNode(Node*front, char*data);

