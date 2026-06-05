#pragma once
using namespace std;

class Goal;

class GoalSelector
{
private:
	class InternalGoal
	{
	public:
		//  canDelete param
		InternalGoal(int prio, Goal *goal, bool canDeletePointer);

		Goal *goal;
		int prio;
		bool canDeletePointer;
	};

private:
	vector<InternalGoal *> goals;
	vector<InternalGoal *> usingGoals;
	int tickCount;
	int newGoalRate;

public:
	GoalSelector();
	~GoalSelector();

	//  canDelete param
	void addGoal(int prio, Goal *goal, bool canDeletePointer = true);
	void tick();
	vector<InternalGoal *> *getRunningGoals();

private:
	bool canContinueToUse(InternalGoal *ig);
	bool canUseInSystem(InternalGoal *goal);
	bool canCoExist(InternalGoal *goalA, InternalGoal *goalB);

public:
	void setNewGoalRate(int newGoalRate);

	//  override to update ai elements when loading entity from schematics
	void setLevel(Level *level);
};