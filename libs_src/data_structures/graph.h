#include <unorderedLinkedList.h>
#include <linkedQueue.h>

using namespace std;

class graphType
{
public:
	bool isEmpty() const;
	//Function to determine whether the graph is empty.
	//Postcondition: Returns true if the graph is empty;
	// otherwise, returns false.
	void createGraph();
	void createGraph(PVOID uLLAddr, linkedListNodeType<int>* uLLNTAddr);
	//Function to create a graph.
	//Postcondition: The graph is created using the
	// adjacency list representation.
	void clearGraph();
	//Function to clear graph.
	//Postcondition: The memory occupied by each vertex
	// is deallocated.
	void printGraph() const;
	//Function to print graph.
	//Postcondition: The graph is printed.
	void depthFirstTraversal();
	//Function to perform the depth first traversal of
	//the entire graph.
	//Postcondition: The vertices of the graph are printed
	// using depth first traversal algorithm.
	void dftAtVertex(int vertex);
	//Function to perform the depth first traversal of
	//the graph at a node specified by the parameter vertex.
	//Postcondition: Starting at vertex, the vertices are
	// printed using depth first traversal
	// algorithm.
	void breadthFirstTraversal();
	//Function to perform the breadth first traversal of
	//the entire graph.
	//Postcondition: The vertices of the graph are printed
	// using breadth first traversal algorithm.
	graphType(int size = 0);
	//Constructor
	//Postcondition: gSize = 0; maxSize = size;
	// graph is an array of pointers to linked
	// lists.
	~graphType();
	//Destructor
	//The storage occupied by the vertices is deallocated.
protected:
	int maxSize; //maximum number of vertices
	int gSize; //current number of vertices
	unorderedLinkedList<int>* graph; //array to create
	 //adjacency lists
private:
	void dft(int v, bool visited[]);
	//Function to perform the depth first traversal of
	//the graph at a node specified by the parameter vertex.
	//This function is used by the public member functions
	//depthFirstTraversal and dftAtVertex.
	//Postcondition: Starting at vertex, the vertices are
	// printed using depth first traversal
	// algorithm.
};

bool graphType::isEmpty() const
{
	return (gSize == 0);
}

void graphType::createGraph()
{
	ifstream infile;
	char fileName[50];
	int index;
	int vertex;
	int adjacentVertex;
	if (gSize != 0) //if the graph is not empty, make it empty
		clearGraph();
	cout << "Enter input file name: ";
	cin >> fileName;
	cout << endl;
	infile.open(fileName);
	if (!infile)
	{
		cout << "Cannot open input file." << endl;
		return;
	}
	infile >> gSize; //get the number of vertices
	graph = new unorderedLinkedList<int>[gSize]; //this was missing on the book

	for (index = 0; index < gSize; index++)
	{
		infile >> vertex;
		infile >> adjacentVertex;

		while (adjacentVertex != -999)
		{
			graph[vertex].insertLast(adjacentVertex);
			infile >> adjacentVertex;
		} //end while
	} // end for
	infile.close();
} //end createGraph

void graphType::createGraph(PVOID uLLTAddr, linkedListNodeType<int>* uLLNTAddr)
{
	ifstream infile;
	char fileName[50];
	int index;
	int vertex;
	int adjacentVertex;
	if (gSize != 0) //if the graph is not empty, make it empty
		clearGraph();
	cout << "Enter input file name: ";
	cin >> fileName;
	cout << endl;
	infile.open(fileName);
	if (!infile)
	{
		cout << "Cannot open input file." << endl;
		return;
	}
	infile >> gSize; //get the number of vertices
	graph = new(uLLTAddr) unorderedLinkedList<int>[gSize](); //this was missing on the book
	
	int i = 0;
	for (index = 0; index < gSize; index++)
	{
		infile >> vertex;
		infile >> adjacentVertex;

		while (adjacentVertex != -999)
		{
			graph[vertex].insertLast(adjacentVertex, uLLNTAddr+i);
			infile >> adjacentVertex;
			i++;
		} //end while
	} // end for
	infile.close();
} //end createGraph

void graphType::clearGraph()
{
	int index;
	for (index = 0; index < gSize; index++)
		graph[index].destroyList();
	gSize = 0;
} //end clearGraph

void graphType::printGraph() const
{
	int index;
	for (index = 0; index < gSize; index++)
	{
		cout << index << " ";
		graph[index].print();
		cout << endl;
	}
	cout << endl;
} //end printGraph

  //Constructor
graphType::graphType(int size)
{
	maxSize = size;
	gSize = 0;
	graph = new unorderedLinkedList<int>[size];
}
//Destructor
graphType::~graphType()
{
	clearGraph();
}

void graphType::dft(int v, bool visited[])
{
	visited[v] = true;
	cout << " " << v << " "; //visit the vertex
	linkedListIterator<int> graphIt;
	//for each vertex adjacent to v
	for (graphIt = graph[v].begin(); graphIt != graph[v].end();
		++graphIt)
	{
		int w = *graphIt;
		if (!visited[w])
			dft(w, visited);
	} //end while
} //end dft

void graphType::depthFirstTraversal()
{
	bool* visited; //pointer to create the array to keep
	 //track of the visited vertices
	visited = new bool[gSize];
	int index;
	for (index = 0; index < gSize; index++)
		visited[index] = false;
	//For each vertex that is not visited, do a depth
	//first traverssal
	for (index = 0; index < gSize; index++)
		if (!visited[index])
			dft(index, visited);
	delete[] visited;
} //end depthFirstTraversal

void graphType::dftAtVertex(int vertex)
{
	bool* visited;
	visited = new bool[gSize];
	for (int index = 0; index < gSize; index++)
		visited[index] = false;
	dft(vertex, visited);
	delete[] visited;
} // end dftAtVertex

void graphType::breadthFirstTraversal()
{
	linkedQueueType<int> queue;
	bool* visited;
	visited = new bool[gSize];
	for (int ind = 0; ind < gSize; ind++)
		visited[ind] = false; //initialize the array
		//visited to false
	linkedListIterator<int> graphIt;
	for (int index = 0; index < gSize; index++)
		if (!visited[index])
		{
			queue.addQueue(index);
			visited[index] = true;
			cout << " " << index << " ";

			while (!queue.isEmptyQueue())
			{
				int u = queue.front();
				queue.deleteQueue();
				for (graphIt = graph[u].begin();
					graphIt != graph[u].end(); ++graphIt)
				{
					int w = *graphIt;
					if (!visited[w])
					{
						queue.addQueue(w);
						visited[w] = true;
						cout << " " << w << " ";
					}
				}
			} //end while
		}
	delete[] visited;
} //end breadthFirstTraversal


class weightedGraphType : public graphType
{
public:
	void createWeightedGraph();
	//Function to create the graph and the weight matrix.
	//Postcondition: The graph using adjacency lists and
	// its weight matrix is created.
	void shortestPath(int vertex);
	//Function to determine the weight of a shortest path
	//from vertex, that is, source, to every other vertex
	//in the graph.
	//Postcondition: The weight of the shortest path from
	// vertex to every other vertex in the
	// graph is determined.
	void printShortestDistance(int vertex);
	//Function to print the shortest weight from vertex
	//to the other vertex in the graph.
	//Postcondition: The weight of the shortest path from
	// vertex to every other vertex in the
	// graph is printed.
	weightedGraphType(int size = 0);
	//Constructor
	//Postcondition: gSize = 0; maxSize = size;
	// graph is an array of pointers to linked
	// lists.
	// weights is a two-dimensional array to
	// store the weights of the edges.
	// smallestWeight is an array to store the
	// smallest weight from source to vertices.
	~weightedGraphType();
	//Destructor
	//The storage occupied by the vertices and the arrays
	//weights and smallestWeight is deallocated.

protected:
	double** weights; //pointer to create weight matrix
	double* smallestWeight; //pointer to create the array to
	//store the smallest weight from
	//source to vertices
};

void weightedGraphType::shortestPath(int vertex)
{
	for (int j = 0; j < gSize; j++)
		smallestWeight[j] = weights[vertex][j];
	bool* weightFound;
	weightFound = new bool[gSize];
	for (int j = 0; j < gSize; j++)
		weightFound[j] = false;
	weightFound[vertex] = true;
	smallestWeight[vertex] = 0;
	for (int i = 0; i < gSize - 1; i++)
	{
		double minWeight = DBL_MAX;
		int v;

		for (int j = 0; j < gSize; j++)
			if (!weightFound[j])
				if (smallestWeight[j] < minWeight)
				{
					v = j;
					minWeight = smallestWeight[v];
				}
		weightFound[v] = true;
		for (int j = 0; j < gSize; j++)
			if (!weightFound[j])
				if (minWeight + weights[v][j] < smallestWeight[j])
					smallestWeight[j] = minWeight + weights[v][j];
	} //end for
} //end shortestPath

void weightedGraphType::printShortestDistance(int vertex)
{
	cout << "Source Vertex: " << vertex << endl;
	cout << "Shortest Distance from Source to each Vertex."
		<< endl;
	cout << "Vertex Shortest_Distance" << endl;
	for (int j = 0; j < gSize; j++)
		cout << setw(4) << j << setw(12) << smallestWeight[j]
		<< endl;
	cout << endl;
} //end printShortestDistance
 //Constructor
weightedGraphType::weightedGraphType(int size)
	:graphType(size)
{
	weights = new double* [size];
	for (int i = 0; i < size; i++)
		weights[i] = new double[size];
	smallestWeight = new double[size];
}

//Destructor
weightedGraphType::~weightedGraphType()
{
	for (int i = 0; i < gSize; i++)
		delete[] weights[i];
	delete[] weights;
	delete smallestWeight;
}

class msTreeType : public graphType
{
public:
	void createSpanningGraph();
	//Function to create the graph and the weight matrix.
	//Postcondition: The graph using adjacency lists and
	// its weight matrix is created.
	void minimalSpanning(int sVertex);
	//Function to create a minimal spanning tree with
	//root as sVertex.
	// Postcondition: A minimal spanning tree is created.
	// The weight of the edges is also
	// saved in the array edgeWeights.
	void printTreeAndWeight();
	//Function to output the edges of the minimal
	//spanning tree and the weight of the minimal
	//spanning tree.
	//Postcondition: The edges of a minimal spanning tree
	// and their weights are printed.
	msTreeType(int size = 0);
	//Constructor
	//Postcondition: gSize = 0; maxSize = size;
	// graph is an array of pointers to linked
	// lists.
	// weights is a two-dimensional array to

	// store the weights of the edges.
	// edges is an array to store the edges
	// of a minimal spanning tree.
	// egdeWeight is an array to store the
	// weights of the edges of a minimal
	// spanning tree.
	~msTreeType();
	//Destructor
	//The storage occupied by the vertices and the arrays
	//weights, edges, and edgeWeights is deallocated.
protected:
	int source;
	double** weights;
	int* edges;
	double* edgeWeights;
};

void msTreeType::minimalSpanning(int sVertex)
{
	int startVertex, endVertex;
	double minWeight;
	source = sVertex;
	bool* mstv;
	mstv = new bool[gSize];
	for (int j = 0; j < gSize; j++)
	{
		mstv[j] = false;
		edges[j] = source;
		edgeWeights[j] = weights[source][j];
	}
	mstv[source] = true;
	edgeWeights[source] = 0;
	for (int i = 0; i < gSize - 1; i++)
	{
		minWeight = DBL_MAX;
		for (int j = 0; j < gSize; j++)
			if (mstv[j])
				for (int k = 0; k < gSize; k++)
					if (!mstv[k] && weights[j][k] < minWeight)

					{
						endVertex = k;
						startVertex = j;
						minWeight = weights[j][k];
					}
		mstv[endVertex] = true;
		edges[endVertex] = startVertex;
		edgeWeights[endVertex] = minWeight;
	} //end for
} //end minimalSpanning

void msTreeType::printTreeAndWeight()
{
	double treeWeight = 0;
	cout << "Source Vertex: " << source << endl;
	cout << "Edges Weight" << endl;
	for (int j = 0; j < gSize; j++)
	{
		if (edges[j] != j)
		{
			treeWeight = treeWeight + edgeWeights[j];
			cout << "(" << edges[j] << ", " << j << ") "
				<< edgeWeights[j] << endl;
		}
	}
	cout << endl;
	cout << "Minimal Spanning Tree Weight: "
		<< treeWeight << endl;
} //end printTreeAndWeight

//Constructor
msTreeType::msTreeType(int size)
	:graphType(size)
{
	weights = new double* [size];
	for (int i = 0; i < size; i++)
		weights[i] = new double[size];
	edges = new int[size];
	edgeWeights = new double[size];
}

//Destructor
msTreeType::~msTreeType()
{
	for (int i = 0; i < gSize; i++)
		delete[] weights[i];
	delete[] weights;
	delete[] edges;
	delete edgeWeights;
}

