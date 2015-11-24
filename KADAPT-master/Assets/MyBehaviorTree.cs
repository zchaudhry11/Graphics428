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
    private bool caught = false;
    private bool greet = false;
    private bool rob = false;

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
                Debug.Log("Oh fuck, the fuzz! They're about " + temp + " ft away!");
                caught = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "lawman")
        {
            if (temp < 75)
            {
                Debug.Log("Hello there fellow");
                greet = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "")
        {
            if (temp < 75)
            {
                Debug.Log("Howdy copper!");
                greet = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }

        temp = ReturnClosestBandit();

        if (occupation == "bandit")
        {
            if (temp < 75)
            {
                Debug.Log("Heheheh");
                greet = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "lawman")
        {
            if (temp < 75)
            {
                Debug.Log("GET OVER HERE YOU RUFFIAN");
                caught = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "")
        {
            if (temp < 75)
            {
                Debug.Log("Oh no!");
                rob = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }

        temp = ReturnClosestWanderer();

        if (occupation == "bandit")
        {
            if (temp < 75)
            {
                Debug.Log("Gimme monies plz");
                rob = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "lawman")
        {
            if (temp < 75)
            {
                Debug.Log("Hello there fellow");
                greet = true;
                //return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
        }
        if (occupation == "")
        {
            if (temp < 75)
            {
                Debug.Log("Howdy friend!");
                greet = true;
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
            if (caught)
            {
                Debug.Log("Bandit - Oh fuck, the fuzz! About " + ReturnClosestLawman() + " away");
                return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
            if (greet) {
                Debug.Log("Bandit - Heheheh");
            }
            if (rob)
            {
                Debug.Log("Bandit - Gimme monie plz");
            }
            return new DecoratorLoop(
                new SequenceShuffle(
                    this.ST_ApproachAndWait(this.wander1),
                    this.ST_ApproachAndWait(this.wander2),
                    this.ST_ApproachAndWait(this.wander3),
                    this.ST_ApproachAndWait(this.wander4)
                )
            );
        }

        else if(occupation == "law")
        {
            if (caught)
            {
                Debug.Log("Police - Found you ruffian! About " + ReturnClosestBandit() + " away");
                caught = false;
                return new DecoratorLoop(new Sequence(this.ST_ApproachAndWait(this.wander1)));
            }
            if (greet)
            {
                Debug.Log("Police - Hello there");
                greet = false;
            }
            if (rob)
            {
                Debug.Log("Police - Wait what");
                rob = false;
            }
            return new DecoratorLoop(
                new SequenceShuffle(
                    this.ST_ApproachAndWait(this.wander1),
                    this.ST_ApproachAndWait(this.wander2),
                    this.ST_ApproachAndWait(this.wander3),
                    this.ST_ApproachAndWait(this.wander4)
                )
            );
        }

        else
        {
            if (greet)
            {
                Debug.Log("Civilian - Hi There!");
                greet = false;
            }
            if (rob)
            {
                Debug.Log("Civilian - Oh no!");
                rob = false;
            }
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
