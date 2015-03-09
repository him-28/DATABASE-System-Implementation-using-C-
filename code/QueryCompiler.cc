#include "QueryCompiler.h"
#include "QueryOptimizer.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"
#include "DBFile.h"
#include "Comparison.h"
#include "Function.h"
#include "RelOp.h"

using namespace std;


QueryCompiler::QueryCompiler(Catalog& _catalog, QueryOptimizer& _optimizer) :
	catalog(&_catalog), optimizer(&_optimizer) {	
}

QueryCompiler::~QueryCompiler() {
}

void QueryCompiler::Compile(TableList* _tables, NameList* _attsToSelect,
	FuncOperator* _finalFunction, AndList* _predicate,
	NameList* _groupingAtts, int& _distinctAtts,
	QueryExecutionTree& _queryTree) {

	// tables = FROM
	// attstoselect = SELECT
	// if finalfunc != Null .. either a sum, or group by
	// if groupingatts == null, then its just a sum, else its a groupby
	
	// create a SCAN operator for each table in the query 
	
	TableList* tables = _tables;

	vector <Schema> schemaz;
	vector <Scan> scanz;	
	vector <Select> selectz;
	
	while (tables != NULL)
	{
		DBFile db;
		Schema sch;
		string tab(tables->tableName);
		catalog->GetSchema(tab, sch);
		schemaz.push_back(sch);
		Scan scan(sch,db);
		scanz.push_back(scan);
		tables = tables->next;
	}

	// push-down selections: create a SELECT operator wherever necessary

	for (int i=0; i<schemaz.size(); i++)
	{
		CNF cnf;
	
		Schema sch = schemaz[i];
		Record rec;
		RelationalOp* producer;

		cnf.ExtractCNF (*_predicate, sch, rec);
		Select sel(sch, cnf , rec ,producer);
		selectz.push_back(sel);
                //cout<<scanz[i];

	}

	// call the optimizer to compute the join order
	OptimizationTree* root;
	optimizer->Optimize(_tables, _predicate, root);

	cout<<"\n\nDisplaying Tree\n\n";
	optimizer->treeDisp(root);


         for (int i=0; i< root->tables.size();i++)
	{
		cout<<scanz[i]<<selectz[i]<<optimizer->treeDisp(root)<<endl;

	}
	// create join operators based on the optimal order computed by the optimizer

	//for (int i = 0; i < scanz.size(); i++) cout<<selectz[i];

	// create the remaining operators based on the query

	// connect everything in the query execution tree and return

	// free the memory occupied by the parse tree since it is not necessary anymore
}
