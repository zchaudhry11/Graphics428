using UnityEngine;
using TreeSharpPlus;

public class WanderBehavior : MonoBehaviour 
{
    public Transform wander1;
    public Transform wander2;
    public Transform wander3;
    public GameObject participant;

    private BehaviorAgent behaviorAgent;

    void Start () 
	{
        behaviorAgent = new BehaviorAgent(this.BuildTreeRoot());
        BehaviorManager.Instance.Register(behaviorAgent);
        behaviorAgent.StartBehavior();
	}

    protected Node ST_ApproachAndWait(Transform target)
    {
        Val<Vector3> position = Val.V( () => target.position);
        return new Sequence(participant.GetComponent<BehaviorMecanim>().Node_GoTo(position), new LeafWait(1000));
    }

    private Node BuildTreeRoot()
    {
        return 
            new DecoratorLoop(
                new SequenceShuffle(
                    this.ST_ApproachAndWait(this.wander1), 
                    this.ST_ApproachAndWait(this.wander2), 
                    this.ST_ApproachAndWait(this.wander3) ));
    }

}