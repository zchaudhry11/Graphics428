using UnityEngine;
using System.Collections;
using TreeSharpPlus;

public class MyBehaviorTree : MonoBehaviour
{
	public Transform wander1;
	public Transform wander2;
	public Transform wander3;
    public Transform wander4;
    public Transform wander5;

    public string occupation;

    public GameObject participant;

	private BehaviorAgent behaviorAgent;
	// Use this for initialization
	void Start ()
	{
		behaviorAgent = new BehaviorAgent (this.BuildTreeRoot ());
		BehaviorManager.Instance.Register (behaviorAgent);
		behaviorAgent.StartBehavior ();
	}

    // Update is called once per frame
    void Update()
    {
        double temp = ReturnClosestLawman();
        //Debug.Log(ReturnClosestLawman());
        if (occupation == "bandit")
        {
            if (temp < 75)
            {
                Debug.Log("Oh fuck, the fuzz! They're about "+temp+" ft away!");
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "lawman")
        {
            if (temp < 75)
            {
                //Debug.Log("Hello there fellow");
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "")
        {
            if (temp < 75)
            {
                //Debug.Log("The police!");
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
    }

	protected Node ST_ApproachAndWait(Transform target)
	{
		Val<Vector3> position = Val.V (() => target.position);
        return new Sequence(participant.GetComponent<BehaviorMecanim>().Node_GoTo(position), new LeafWait(1000));
	}

    double ReturnClosestLawman()
    {
        GameObject[] neighbors;
        neighbors = GameObject.FindGameObjectsWithTag("Lawman");
        float distance = Mathf.Infinity;
        Vector3 position = transform.position;
        foreach (GameObject neighbor in neighbors)
        {
            Vector3 diff = neighbor.transform.position - position;
            float curDistance = diff.sqrMagnitude;
            if (curDistance < distance)
            {
                distance = curDistance;
            }
        }
        return distance;
    }

    double ReturnClosestBandit()
    {
        GameObject[] neighbors;
        neighbors = GameObject.FindGameObjectsWithTag("Bandit");
        float distance = Mathf.Infinity;
        Vector3 position = transform.position;
        foreach (GameObject neighbor in neighbors)
        {
            Vector3 diff = neighbor.transform.position - position;
            float curDistance = diff.sqrMagnitude;
            if (curDistance < distance)
            {
                distance = curDistance;
            }
        }
        return distance;
    }

    double ReturnClosestWanderer()
    {
        GameObject[] neighbors;
        neighbors = GameObject.FindGameObjectsWithTag("Wanderer");
        float distance = Mathf.Infinity;
        Vector3 position = transform.position;
        foreach (GameObject neighbor in neighbors)
        {
            Vector3 diff = neighbor.transform.position - position;
            float curDistance = diff.sqrMagnitude;
            if (curDistance < distance)
            {
                distance = curDistance;
            }
        }
        return distance;
    }

    protected Node BuildTreeRoot()
	{
		
        if (occupation == "bandit")
        {
            if (ReturnClosestLawman() < 75)
            {
                Debug.Log("Oh fuck, the fuzz! About " + ReturnClosestLawman() + " away");
                return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
            else {
                return new DecoratorLoop(
                    new SequenceShuffle(
                        this.ST_ApproachAndWait(this.wander1),
                        this.ST_ApproachAndWait(this.wander2),
                        this.ST_ApproachAndWait(this.wander3),
                        this.ST_ApproachAndWait(this.wander4)
                     )
                );
            }
        }

        else if(occupation == "law")
        {
            if (ReturnClosestBandit() < 75)
            {
                Debug.Log("Gon git dat bandit! About "+ReturnClosestBandit()+" away");
                return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander2)));
            }
            else {
                return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }

        else
        {

            if (ReturnClosestBandit() < 75)
            {
                Debug.Log("Oh no, a bandit! About " + ReturnClosestBandit() + " away");
                return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));

            }
            else {
                return new DecoratorLoop(
                    new SequenceShuffle(
                        this.ST_ApproachAndWait(this.wander1),
                        this.ST_ApproachAndWait(this.wander2),
                        this.ST_ApproachAndWait(this.wander3),
                        this.ST_ApproachAndWait(this.wander4),
                        this.ST_ApproachAndWait(this.wander5)
                     )
                );
            }
        }
	}
}
